# COMP345_T8

## Running the drivers (Windows)

Run these commands from the repository root in Command Prompt.

### Map
```cmd
g++ MapDriver.cpp Map.cpp Player.cpp Cards.cpp Orders.cpp -o map
map.exe
```

### Player
```cmd
g++ PlayerDriver.cpp Player.cpp Cards.cpp Orders.cpp Map.cpp -o player
player.exe
```

### Orders
```cmd
g++ Orders.cpp OrdersDriver.cpp Map.cpp Player.cpp Cards.cpp -o orders
orders.exe
```

### Cards
```cmd
g++ Cards.cpp CardsDriver.cpp Player.cpp Orders.cpp Map.cpp -o cards
cards.exe
```

### Game Engine
```cmd
g++ GameEngine.cpp GameEngineDriver.cpp -o game_engine
game_engine.exe
```
