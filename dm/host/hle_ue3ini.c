// Unreal Engine 3's own picture settings from the launchers (profile key ue3_ini, DM_GAME_UE3_INI; Batman: Arkham Knight). The engine reads its configuration from a coalesced ini in the RomFS
// (BmGame/CookedSwitch/Coalesced_<LANG>.bin: every .ini of the game in one file). When the game opens one, the host serves a copy in DM_TEMP with these values set in every section of that name:
//   DM_RENDER_SCALE (0.5 to 1.0)  [SystemSettings] ScreenPercentage, [DynamicResolution] MinScreenPercentage and MaxScreenPercentage = scale × 100. Arkham Knight's Switch port locks its 3D
//                                 scene to 75 % of the screen (MinScreenPercentage 75 with bLockedToMin) and scales it up
//   DM_AA (0 / 1)                 [SystemSettings] bAllowPostprocessMLAA: the engine's post-process anti-aliasing (Arkham Knight: Rocksteady's SMAA, off in the Switch's settings, on in its higher buckets)
//   DM_TEX_FILTER (2 to 16)       [SystemSettings] MaxAnisotropy (Arkham Knight's Switch settings: 4); nvn_mtl.m raises the Metal samplers to it as well
// A variable that is not set leaves the game's value. The texture LOD settings are never touched: the Switch cook has no mips past its MaxLODSize (raising it scrambled Arkham Knight's textures).
// Format: i32 file count, then per file {FString name, i32 section count, per section {FString name, i32 key count, per key {FString key, FString value}}}; an FString is an i32 length counting the
// NUL (negative: UTF-16 code units), then the characters
#include "hle.h"
#include <sys/stat.h>

#if DM_GAME_UE3_INI
typedef struct { const char *sec, *key; char val[32]; } ue3_set;

static int fstr(const uint8_t *d, size_t n, size_t *o, const uint8_t **s, int32_t *len) {   // one FString at *o: its bytes and length field
    if (*o + 4 > n) return 0;
    memcpy(len, d + *o, 4); *o += 4; *s = d + *o;
    size_t b = *len < 0 ? (size_t)(-(int64_t)*len) * 2 : (size_t)*len;
    if (*o + b > n) return 0;
    *o += b; return 1;
}
static int fstr_is(const uint8_t *s, int32_t len, const char *a) {   // FString equals the ASCII string a
    size_t k = strlen(a);
    if (len > 0) return (size_t)len == k + 1 && !memcmp(s, a, k);
    if (len < 0 && (size_t)(-(int64_t)len) == k + 1) { for (size_t i = 0; i < k; i++) if (s[2 * i] != (uint8_t)a[i] || s[2 * i + 1]) return 0; return 1; }
    return 0;
}
static int i32(const uint8_t *d, size_t n, size_t *o, int32_t *v) { if (*o + 4 > n) return 0; memcpy(v, d + *o, 4); *o += 4; return 1; }
static void put(uint8_t **w, const void *p, size_t k) { memcpy(*w, p, k); *w += k; }
// The rewritten file into out (room for n + 64 per value set); returns its length, 0 when the file does not parse
static size_t rewrite(const uint8_t *d, size_t n, uint8_t *out, const ue3_set *set, int nset, int *changed) {
    size_t o = 0; int32_t nf, ns, nk, len; const uint8_t *s; uint8_t *w = out;
    if (!i32(d, n, &o, &nf) || nf < 0 || nf > 4096) return 0;
    put(&w, &nf, 4);
    for (int f = 0; f < nf; f++) {
        size_t a = o; if (!fstr(d, n, &o, &s, &len) || !i32(d, n, &o, &ns)) return 0;
        put(&w, d + a, o - a);
        for (int x = 0; x < ns; x++) {
            const uint8_t *sn; int32_t sl; a = o;
            if (!fstr(d, n, &o, &sn, &sl) || !i32(d, n, &o, &nk)) return 0;
            put(&w, d + a, o - a);
            for (int k = 0; k < nk; k++) {
                const uint8_t *kn; int32_t kl; size_t ka = o;
                if (!fstr(d, n, &o, &kn, &kl)) return 0;
                size_t va = o; if (!fstr(d, n, &o, &s, &len)) return 0;
                const ue3_set *hit = NULL;
                for (int i = 0; i < nset && !hit; i++) if (set[i].val[0] && fstr_is(sn, sl, set[i].sec) && fstr_is(kn, kl, set[i].key)) hit = &set[i];
                put(&w, d + ka, va - ka);
                if (!hit) { put(&w, d + va, o - va); continue; }
                int32_t nl = (int32_t)strlen(hit->val) + 1; put(&w, &nl, 4); put(&w, hit->val, (size_t)nl);
                (*changed)++;
            }
        }
    }
    return o == n ? (size_t)(w - out) : 0;
}

// RomFS host path → the copy to open instead (in out), when it is a coalesced ini and a setting applies; 0 = open the file itself
int ue3ini_path(const char *path, char *out, size_t outn) {
    const char *b = strrchr(path, '/'); b = b ? b + 1 : path;
    size_t bl = strlen(b);
    if (strncmp(b, "Coalesced_", 10) || bl < 14 || strcmp(b + bl - 4, ".bin")) return 0;
    static ue3_set set[5]; static int nset = -1; static pthread_mutex_t mu = PTHREAD_MUTEX_INITIALIZER;
    pthread_mutex_lock(&mu);
    if (nset < 0) {
        nset = 0; const char *e;
        if ((e = getenv("DM_RENDER_SCALE")) && *e) {
            char *end; float sc = strtof(e, &end);
            if (end == e || *end || !(sc >= 0.5f && sc <= 1.0f)) LOG("[RED] DM_RENDER_SCALE=%s: must be 0.5 to 1.0; keeping the game's screen percentage", e);
            else { const char *ks[3][2] = {{"SystemSettings", "ScreenPercentage"}, {"DynamicResolution", "MinScreenPercentage"}, {"DynamicResolution", "MaxScreenPercentage"}};
                   for (int i = 0; i < 3; i++) { set[nset].sec = ks[i][0]; set[nset].key = ks[i][1]; snprintf(set[nset].val, sizeof set[0].val, "%.1f", sc * 100); nset++; } }
        }
        if ((e = getenv("DM_AA")) && *e) { set[nset].sec = "SystemSettings"; set[nset].key = "bAllowPostprocessMLAA"; strlcpy(set[nset].val, atoi(e) ? "True" : "False", sizeof set[0].val); nset++; }
        if ((e = getenv("DM_TEX_FILTER")) && atoi(e) >= 2) { set[nset].sec = "SystemSettings"; set[nset].key = "MaxAnisotropy"; snprintf(set[nset].val, sizeof set[0].val, "%d", atoi(e) > 16 ? 16 : atoi(e)); nset++; }
    }
    int ok = 0;
    if (nset) {
        char dir[1024], dst[1024]; const char *t = getenv("DM_TEMP");
        snprintf(dir, sizeof dir, "%s/ue3ini", t && *t ? t : "/tmp"); mkdir(dir, 0755);
        snprintf(dst, sizeof dst, "%s/%s", dir, b);
        static char made[8][64]; static int nmade;   // the copies made in this run (a copy left by an earlier run may hold other settings)
        for (int i = 0; i < nmade && !ok; i++) if (!strcmp(made[i], b)) ok = 1;
        if (!ok) {
            FILE *f = fopen(path, "rb"); uint8_t *d = NULL, *o2 = NULL; size_t n = 0;
            if (f && !fseek(f, 0, SEEK_END) && (n = (size_t)ftell(f)) > 4 && !fseek(f, 0, SEEK_SET) && (d = malloc(n)) && fread(d, 1, n, f) == n && (o2 = malloc(n + 64 * 4096))) {
                int changed = 0; size_t m = rewrite(d, n, o2, set, nset, &changed);
                char tmp[1100]; snprintf(tmp, sizeof tmp, "%s.partial", dst);
                FILE *g = m ? fopen(tmp, "wb") : NULL;
                if (g && fwrite(o2, 1, m, g) == m && !fclose(g) && !rename(tmp, dst)) {
                    ok = 1; if (nmade < 8) strlcpy(made[nmade++], b, sizeof made[0]);
                    LOG("ue3 ini: %s served with %d value(s) set from the launcher's settings (%s)", b, changed, dst); }
                else { if (g) fclose(g); LOG("[RED] ue3 ini: %s could not be rewritten (%s); the game reads its own settings", b, m ? "write failed" : "not a coalesced ini of the known form"); }
            }
            if (f) fclose(f);
            free(d); free(o2);
        }
        if (ok) strlcpy(out, dst, outn);
    }
    pthread_mutex_unlock(&mu);
    return ok;
}
#else
int ue3ini_path(const char *path, char *out, size_t outn) { (void)path; (void)out; (void)outn; return 0; }
#endif
