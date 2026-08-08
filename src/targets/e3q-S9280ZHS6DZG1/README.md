# e3q-S9280ZHS6DZG1 offline compatibility status

This profile targets Hong Kong/TGY Galaxy S24 Ultra firmware
`SM-S9280 / S9280ZHS6DZG1` only.

Verified input identity:

```text
device: e3q
product: e3qzhx
boot fingerprint: samsung/e3qzhx/e3q:14/UP1A.231005.007/S9280ZHS6DZG1:user/release-keys
kernel: 6.1.145-android14-11-33419968-abS9280ZHS6DZG1
boot.img SHA-256: badc002ec9ca7e9a4861925c52b318cd943b13a81e8ebee408fe0908908e4d96
kernel SHA-256: 826a9d3769ff8aab02fca3cca85cdebf84415052687d418cca07dd4e1d4b4792
BTF SHA-256: 8415104c012e18942b18bcb52f401075cb6b92df837b9552a8c11070d65efe56
```

The detached BTF is byte-identical to the hash-frozen
`e3q-S928USQS6DZF2` BTF. Every required named symbol has the same address,
and all 256 P0 samples were compared against the DZF2 table. Exactly one
sample differs: row `0x150000`, page offset `0x0200`. The derived header
therefore inherits the audited E3Q constants and overrides only that sample
and the exact build identity.

The matching DZG1 BL and vendor boot images were subsequently verified:

```text
BL archive SHA-256: 0fe032eb6e0c7e1f32ae27132b12c7187d19101931d81898260405a4e2d2d17a
abl.elf SHA-256: 5402bae2c5f97adcd462f896044bf5995ccb82bce1babb64c37e0084035f9db5
LinuxLoader PE SHA-256: 9982040516c756a6ff2445357d56e96669e8f7b25b9b1b413854d212e5e94c02
vendor_boot.img SHA-256: 939a045e8cb91ae0a0739531363e3eef86c65f0bb0678059f06e0d65698eaac5
```

The DZG1 LinuxLoader computes the ARM64 load base at RVA `0x176f8` through
`0x17740`. Its selected constants are `0x00080000` and `0x05600000`, and the
entry calculation at RVA `0x178cc` through `0x178f0` adds the raw Image
`text_offset`. The final ARM64 handoff loads the entry at RVA `0x18d48` and
branches through it at RVA `0x18da4`. All 22 vendor-boot DTBs independently
contain `gunyah_hyp_region@80000000` with
`reg = <0 0x80000000 0 0x00e00000>`. With the target Image's zero
`text_offset`, this confirms:

```c
#define P0_PHYS_OFFSET      0x80000000ULL
#define P0_KERNEL_PHYS_LOAD 0x80080000ULL
```

The release app payload was built twice with the official Windows NDK r29
(`29.0.14206865`). Both builds reproduced byte-for-byte:

```text
unpadded size: 89144
fixed release size: 104128
SHA-256: 87852d959cd7c08d0edfc626f24b981c970e1c115ea8a8a98e1b20b7371111c7
```

The AArch64 ELF and its dynamic dependencies were checked with the NDK
`llvm-readelf`.

This is an offline profile, not a hardware-validated payload. Do not add it
to the support feed until the release payload and exact-vermagic KernelSU
artifacts have been built and the exploit has been tested on authorized
SM-S9280 hardware.

## Hardware validation status (2026-08)

Device: SM-S9280 (Galaxy S24 Ultra, TGY) running firmware
`S9280ZHS6DZG1` (Android 14, kernel 6.1.145-android14-11-33419968).

### Iteration A0 - offline baseline (SHA `87852d95...1111c7`)

Built with NDK r29 (`29.0.14206865`) using the unmodified DZF2-inherited
constants. Deployed via Root My Galaxy debug build:
`Root-My-Galaxy-SM-S9280-DZG1-ezn24-debug.apk`.

24 attempts:

- 22/24 failed at `pipe KernelSnitch sk_buff page leak`.
- 2/24 reached `p0 pipe oracle prepared`, but the recovered base was
  `ffffff8a0c920000` / `ffffff8812678000` - pages in the kernel direct
  map, **not** in the P0 slide region (`0xffffff8000000000`-`0xffffff801f0000`).
- `KernelSnitch mm_struct leak failed` and `slide kaslr leak failed` on
  every attempt.

Root cause hypothesis: the S928U DZF2 default `P0_ORACLE_PROBE_OFFSET`
of `0x1f0000` lands on the slide-region boundary, which DZG1's slab
allocator evicts before the leak can read it back.

### Iteration A1 - widened KSNITCH window (SHA `7597a8e2...7248`, rolled back)

Override applied:

```c
#define P0_ORACLE_PROBE_OFFSET 0x100000ULL
#define SLIDE_KSNITCH_APPENDED_FUTEXES 4096
#define SLIDE_KSNITCH_REPEAT_MEASUREMENT 128
#define SLIDE_KSNITCH_AVERAGE 16
```

Result: 24/24 attempts failed at startup with
`SYSCHK(sched_setaffinity): Invalid argument` (kernelsnitch/utils.h:142).
The KSNITCH hypothesis was incorrect - iteration A2 has the same failure
without the KSNITCH override (see below). The current leading suspect is
the Linux NDK producing a different code path from the Windows NDK.

### Iteration A2 - probe page only (current, pending)

Override applied:

```c
#define P0_ORACLE_PROBE_OFFSET 0x100000ULL
```

KSNITCH parameters remain at DZF2 defaults. Result: 24/24 attempts
failed at startup with the same `sched_setaffinity: Invalid argument`.
Since the KSNITCH override is absent in A2, that parameter set is ruled
out as the cause. Two suspects remain: the `P0_ORACLE_PROBE_OFFSET`
change itself, or the Linux NDK build. Next step: rebuild A0 (no
override) with the Linux NDK to isolate which one is responsible. See
`outputs/SM-S9280-DZG1-exploit-A2.log` for the raw attempts.
