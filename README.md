Flare Ensign is a project started as a learning tool for C++ following Javidx9's videos, aka OneLoneCoder, making use of his olcPixelGameEngine.

OneLoneCoder's YouTube channel: https://www.youtube.com/@javidx9

This particular project was about the implementation of A* pathfinding algorithms, so I choose to make a rendition of Fire Emblem, a game series I am very fond of (particularly the GBA games).

<img width="1438" height="747" alt="pathfinding" src="https://github.com/user-attachments/assets/122c813f-c954-43be-8d53-308595881060" />
From the very beginning I wanted to make this snippet of a game to be easily editable, that's why instead of fixed backgrounds it uses a tilemap image and CSV files to draw the map. 
The map can be edited with Tiled (https://www.mapeditor.org/) which can export as CSV. The units and jobs (aka RPG classes) are implemented as JSON, also for easy editing. 

I added support for JSON later in the coding, that's why I don't just read Tiled's TMJ files which are just JSON with a different extensions. Maybe I'll do that later. <br>

In the process of making this example project I included the following features:
- Implementation of the A* algorithm, including tile types for determining blocked tiles or tiles that require extra effort to traverse (like forest tiles) 
- Incorporating cubic interpolation look-up tables for smooth displacement of the screen and some HUD elements.
- Rudimentary 2d animation system based on tiled bitmaps.
- Movement and combat routines.
- Facilities for creating user interfaces, including variable size windows, bitmap based fonts and animated bars 

This project uses:
- OneLoneCoder's olcPixelGameEngine
  https://github.com/OneLoneCoder/olcPixelGameEngine
- Niels Lohman's JSON library
  https://github.com/nlohmann/json
- Free graphic assets by Shade (tile and unit graphics)
  https://merchant-shade.itch.io/
- Free graphic assets by BDradrong1727 (animated cursor, windows and bar graphics )
  https://bdragon1727.itch.io/

<img width="1439" height="749" alt="hud" src="https://github.com/user-attachments/assets/f28d5137-9827-47e4-a5e3-e5c4399c19fb" />


