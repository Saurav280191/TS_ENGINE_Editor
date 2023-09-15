# TS_ENGINE_Editor

Steps to build the editor:
1. Run GenerateVS19Project.bat or GenerateVS22Project for VS19 or VS22 respectively.
2. Run CopyDataToBin. This is delete build\x64\Debug\bin\Asset and build\x64\Debug\bin\Resources if they exist. And place the folders from the current folder to build\x64\Debug\bin. 
3. After that you can run TS_ENGINE_Editor.exe from build\x64\Debug\bin.
4. Import feature is not implemented yet. So if you want to place you own assets you can place it in build\x64\Debug\bin\Asset\Model or Assets\Textures.
5. If you save a scene you will find it in build\x64\Debug\bin\Assets\SavedScenes folder to open it later.

Information:
This is the editor code for my framework TS_ENGINE.
It currently includes some basic editor related features like:
1. GameObject creation.
2. Scene save and load system.
3. Different panels for showing scene hierarchy and transforming position, rotation and scale.
4. Picking and manipulating objects.
5. Material editor. (In progress)
6. HDR color and gamma correction. (In progress)
