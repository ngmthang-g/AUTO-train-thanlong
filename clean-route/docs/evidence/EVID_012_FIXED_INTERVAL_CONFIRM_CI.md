# EVID-012 — v1.5.8 Fixed Interval Confirm Windows CI

Windows x64 staging CI **PASS** — run `31942843638`, job `95154396882`.

Hash-gated source reconstruction:
- transported controller SHA256 `f2a7dc1b5d1a94544011bb56a1f7b8e3df30fc54776fb706ec1c0424c8e00af9`;
- build-only deterministic duplicate-case normalization produced compile controller SHA256 `01cafd0a490a140da10db5a92ac23f7b70d794f88cb4936f002f07962dfb0ca0`;
- Bridge source SHA256 `64967db0d72cd584c909ce12dd4fdf20cace1128784b9e43214bd419014ed05f`.

Validation passed:
- v1.5.8 safety audit;
- route/mount self-test 15/15;
- Bridge DLL compile + PE verification;
- controller EXE compile;
- packaging and NativeLibrary/export verification.

Staging binary hashes:
- EXE `e6862b4aad90c28e272408cde08f1aadea641a7783f63acec381702d2e688f4d`;
- Bridge DLL `5d7adcb842378cdd91225ee476efb5e5e8dbe4bf67929af7afc97af6eb951a24`;
- Source ZIP `1ac66f481138c585099bd1f54f11031c85f1f8bf0f2d3d0c882cb218481add42`.

Evidence boundary: this is BUILD/CI evidence only. Runtime fixed-interval Confirm remains **NEEDS USER TEST**.
