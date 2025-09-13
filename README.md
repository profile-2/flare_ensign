Flare Ensign is a project started a learning tool for C++ following Javidx9's videos, aka OneLoneCoder, making use of his olcPixelGameEngine.

OneLoneCoder's YouTube channel: https://www.youtube.com/@javidx9

This particular project was about the implementation of A* pathfinding algorythms, so I choose to make a rendition of Fire Emblem, a game series I am very fond of (particularly the GBA games).

In the process of making this example project I included the following features:
- Ability to read CSV files produced by Tiled (https://www.mapeditor.org/) for easy editing of background tiles <img width="1617" height="906" alt="tiled" src="https://github.com/user-attachments/assets/56ececa0-c60d-4e8b-a528-790e1a348433" />
- Implementation of the A* algorythm, including tile types for determining blocked tiles or tiles that require extra effort to traverse (like forest tiles) <img width="1438" height="747" alt="pathfinding" src="https://github.com/user-attachments/assets/122c813f-c954-43be-8d53-308595881060" />
- Incorporating cubic interpolation look-up tables for smooth displacement of the screen and some HUD elements.
- Rudimentary 2d animation system based on tiled bitmaps.
- Movement and combat routines.
- Use of JSON files (including schemas) for storing jobs (aka RPG classes) and units.
- Facilities for creating user interfases, including variable size windows, bitmap based fonts and animated bars <img width="1439" height="749" alt="hud" src="https://github.com/user-attachments/assets/f28d5137-9827-47e4-a5e3-e5c4399c19fb" />

This project uses:
- OneLoneCoder's olcPixelGameEngine
  https://github.com/OneLoneCoder/olcPixelGameEngine
- Niels Lohman's JSON library
  https://github.com/nlohmann/json
- Free graphic assets by Shade (tile and unit graphics)
  https://merchant-shade.itch.io/
- Free graphic assets by BDradrong1727 (animated cursor, windows and bar graphics )
  https://bdragon1727.itch.io/


