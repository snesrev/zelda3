# tvOS build

This target builds the game as a native tvOS app using SDL2's UIKit and
GameController backends. The ROM is never committed to this repository.

1. Put an authorized US ROM at `platform/tvos/Resources/zelda3.sfc`.
2. Create the Python environment used by the asset extractor:

   ```sh
   python3 -m venv .venv
   .venv/bin/python -m pip install -r requirements.txt
   ```

3. Extract the runtime assets from that ROM from the repository root:

   ```sh
   .venv/bin/python assets/restool.py --rom platform/tvos/Resources/zelda3.sfc --extract-from-rom
   cp zelda3_assets.dat platform/tvos/Resources/
   ```

4. Generate the Xcode project:

   ```sh
   cmake -S . -B build/tvos -G Xcode \
     -DCMAKE_SYSTEM_NAME=tvOS -DCMAKE_OSX_SYSROOT=appletvos
   open build/tvos/Zelda3.xcodeproj
   ```

5. Select the `Zelda3` scheme, choose your Apple TV, set your Apple Developer
   team/signing identity in Xcode, and Run. The app stages the bundled ROM,
   assets, and config into its writable Application Support directory; save
   states and SRAM are stored there as well.

For a command-line device build, set `CODE_SIGNING_ALLOWED=NO` only for a
compile check. A real Apple TV install requires a development signing team.
