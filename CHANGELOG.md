# Changelog

## [0.2.0](https://github.com/beeping-io/beeping-core/compare/v0.1.0...v0.2.0) (2026-05-08)


### ✨ Features

* **release:** BEE-1696 enable Linux arm64 / Raspberry Pi target ([#21](https://github.com/beeping-io/beeping-core/issues/21)) ([0a1cc80](https://github.com/beeping-io/beeping-core/commit/0a1cc80e3cc981d249a82a727bc1852365f7d92f))
* **release:** BEE-1729 Linux amd64 + Docker compat smoke matrix ([#11](https://github.com/beeping-io/beeping-core/issues/11)) ([d322b4b](https://github.com/beeping-io/beeping-core/commit/d322b4b35a9192b6f7b9f127662c3d30431a994d))
* **release:** BEE-1730 add Windows x64 release with static CRT ([#14](https://github.com/beeping-io/beeping-core/issues/14)) ([1446799](https://github.com/beeping-io/beeping-core/commit/1446799465654e7809954c34deae469fe3c0b8e3))
* **release:** BEE-1732 add Windows ARM64 release with static CRT ([#19](https://github.com/beeping-io/beeping-core/issues/19)) ([1eab603](https://github.com/beeping-io/beeping-core/commit/1eab603ba6815b2ff193d46c5b1d26d6bb5a029a))
* **release:** BEE-2221 Android NDK .so artifacts (16 KB pages) ([#24](https://github.com/beeping-io/beeping-core/issues/24)) ([9821519](https://github.com/beeping-io/beeping-core/commit/98215196af1e1fd0d0d1bb4ee351d602a1f97f56))
* **release:** BEE-2223 publish iOS XCFramework with DWARF symbols ([#25](https://github.com/beeping-io/beeping-core/issues/25)) ([8bda2d1](https://github.com/beeping-io/beeping-core/commit/8bda2d18177e3dd1ac0e93dd68a98de0d537ad5a))
* **wasm:** BEE-1731 add WASM browser + Node decoder (Emscripten) ([#20](https://github.com/beeping-io/beeping-core/issues/20)) ([b581190](https://github.com/beeping-io/beeping-core/commit/b581190c63dad54700e1d9215c267de1da33e3aa))


### 🐛 Bug Fixes

* **ci:** BEE-1696 drop archlinux from arm64 compat-smoke matrix ([#23](https://github.com/beeping-io/beeping-core/issues/23)) ([9ea0546](https://github.com/beeping-io/beeping-core/commit/9ea0546c74eafdea7062c07645892e304273a1ec))
* **ci:** BEE-1696 remove orphan line in arm64 compat-smoke block ([#22](https://github.com/beeping-io/beeping-core/issues/22)) ([8158d77](https://github.com/beeping-io/beeping-core/commit/8158d777554249232f8b05cc32ac3fe35c21852d))
* **ci:** BEE-1730 locate dumpbin via vswhere or skip gracefully ([#17](https://github.com/beeping-io/beeping-core/issues/17)) ([a051180](https://github.com/beeping-io/beeping-core/commit/a0511809823983891fdd9ad9ba6ffc0defa6b22f))
* **ci:** BEE-1730 use absolute install paths in windows-x64 job ([#16](https://github.com/beeping-io/beeping-core/issues/16)) ([328019f](https://github.com/beeping-io/beeping-core/commit/328019fc0166504a91050b97bfa6d87c7f19aa54))
* **release:** BEE-1729 build linux-amd64 on ubuntu-22.04 + static-libstdc++ ([#12](https://github.com/beeping-io/beeping-core/issues/12)) ([0bd18ef](https://github.com/beeping-io/beeping-core/commit/0bd18ef0054478d50e60999e5fede25ce76e6290))
* **release:** BEE-1730 cmake install path for Windows multi-config ([#15](https://github.com/beeping-io/beeping-core/issues/15)) ([de64c17](https://github.com/beeping-io/beeping-core/commit/de64c1706540ab4fedcf06e4a2381fd14507c8ca))
* **release:** BEE-1730 include Windows zip in GitHub Release + cosign ([#18](https://github.com/beeping-io/beeping-core/issues/18)) ([d2c0dba](https://github.com/beeping-io/beeping-core/commit/d2c0dbada760eec2e376c858053c9dc73434bca1))

## [0.1.0](https://github.com/beeping-io/beeping-core/compare/v0.0.0...v0.1.0) (2026-04-23)


### ✨ Features

* **cli:** BEE-1689 beeping-core CLI with decode command ([#6](https://github.com/beeping-io/beeping-core/issues/6)) ([20d5af4](https://github.com/beeping-io/beeping-core/commit/20d5af43a5782a53446e8fdcb4348a0a8c2f79d4))
* **release:** BEE-1694 scope release.yml to macOS-only + iterative per-OS strategy ([#8](https://github.com/beeping-io/beeping-core/issues/8)) ([914835b](https://github.com/beeping-io/beeping-core/commit/914835b3ecf046ddb355f2197b04621086815be4))
