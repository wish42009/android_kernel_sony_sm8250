# XperiaGSI kernel for the Sony Xperia 1 II (pdx203)

Kernel source of the XperiaGSI ROM (Android 17 system from the Xperia 1 VIII on the Xperia 1 II).

Base: [LineageOS/android_kernel_sony_sm8250](https://github.com/LineageOS/android_kernel_sony_sm8250),
branch `lineage-24.0`, commit `7fc8455b2fa84be76364c2457e6503b867ceb12a`.
Every XperiaGSI change is a separate commit on top of it:

| Commit | What | Why |
|---|---|---|
| capability: add CAP_CHECKPOINT_RESTORE | capability 40 + SELinux `cap2` permission | Android 17 userspace and its SELinux policy use it |
| arm64: configs: pdx203: enable uclamp | `CONFIG_UCLAMP_TASK(_GROUP)=y` (drops the unused schedtune) | Android 17 task profiles drive CPU QoS through uclamp only |
| leds: qpnp-flash-v2: 5 mA IRES for torch | torch currents up to 60 mA use the 5 mA step | five flashlight brightness levels instead of 12.5 mA steps |
| sony_camera: BoB load vote for iToF | 2 A load on pm8150a_bob while the ToF module is powered | stops the ToF laser driver fault (lddStatus 0x54), ToF autofocus works |

## Build

Toolchain: AOSP clang `r416183b` (clang 12.0.5), plus `aarch64-linux-gnu-` / `arm-linux-gnueabi-` binutils.

```bash
export ARCH=arm64 SUBARCH=arm64 PATH=/path/to/clang-r416183b/bin:$PATH
ARGS="O=out ARCH=arm64 LLVM=1 LLVM_IAS=1 CC=clang LD=ld.lld AR=llvm-ar NM=llvm-nm OBJCOPY=llvm-objcopy \
      OBJDUMP=llvm-objdump STRIP=llvm-strip READELF=llvm-readelf HOSTCC=clang HOSTCXX=clang++ HOSTLD=ld.lld \
      CROSS_COMPILE=aarch64-linux-gnu- CROSS_COMPILE_ARM32=arm-linux-gnueabi-"

make $ARGS vendor/kona-perf_defconfig
./scripts/kconfig/merge_config.sh -m -O out out/.config \
    arch/arm64/configs/vendor/edo.config arch/arm64/configs/vendor/pdx203.config
make $ARGS olddefconfig
make $ARGS -j$(nproc) Image.gz dtbs modules
```
