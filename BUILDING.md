# Windows 

## Building with Visual Stuio

Dependencies and requirements:
  * The `libsdl2-dev` library which will be automatically installed to Visual Studio with NuGet.
  * [Visual Studio 2022 Community Edition](https://visualstudio.microsoft.com)
  * [Python](https://www.python.org) (*Make sure to add Python to %%PATH%% during the Installation*)
  * A copy of A Link to the Past US ROM with the hash of `66871d66be19ad2c34c927d6b14cd8eb6fc3181965b6e517cb361f7316009cfb`
  * `pillow` and the `pip` python dependencies which is used by the asset extractor

1. Install Visual Studio on your machine.
2. Clone the zelda3 repository. `https://github.com/snesrev/zelda3.git`
3. Rename the copy of your ROM to `zelda3` and place it inside the `tables` directory.

NOTE: *Make sure you're in the `zelda3` directory.*

Open CMD by searching in search bar of Windows and then type `python -m pip install -r requirements.txt` so the python deps will be fully installed.

Go to the `tables` directory and run `extract_resources.py` then `compile_resources.py` to extract the asstes. (This won't be needed in the future since after this the assets will be inside `zelda3_assets.dat`.

Open `Zelda3.sln` and build the solution. 

Now you can run the `zelda3.exe` executabe.
