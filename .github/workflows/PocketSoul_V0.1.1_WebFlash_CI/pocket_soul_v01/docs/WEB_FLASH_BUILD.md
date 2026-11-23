# Web-flash release path (no local ESP-IDF installation required)

This repository includes `.github/workflows/build-firmware.yml` to build in GitHub Actions using Espressif's ESP-IDF CI action.

1. Upload this project to a GitHub repository (or push it to `main`/`master`).
2. Open the repository's **Actions** tab and select **Pocket Soul firmware build**.
3. Run it manually with **Run workflow**, or let it trigger after a push.
4. If the build succeeds, download the workflow artifact named `PocketSoul-firmware-...`.
5. The artifact contains `merged-firmware.bin`. This is the merged image expected by the web-flasher manifest (offset 0).
6. Place the binary in the web installer at `firmware/merged-firmware.bin`, then publish the installer on HTTPS.

Build success is not guaranteed until the workflow is actually run. Hardware flashing and boot behavior still require a test on the target board.
