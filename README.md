# Running the drivers on Windows

Run these commands from the repository's root in Command Prompt.

### Map
```cmd
g++ MapDriver.cpp Map.cpp Player.cpp PlayerStrategies.cpp Cards.cpp Orders.cpp GameEngine.cpp CommandProcessing.cpp LoggingObserver.cpp -o map
map.exe
```

### Player
```cmd
g++ PlayerDriver.cpp Map.cpp Player.cpp PlayerStrategies.cpp Cards.cpp Orders.cpp GameEngine.cpp CommandProcessing.cpp LoggingObserver.cpp -o player
player.exe
```

### Orders
```cmd
g++ OrdersDriver.cpp Map.cpp Player.cpp PlayerStrategies.cpp Cards.cpp Orders.cpp GameEngine.cpp CommandProcessing.cpp LoggingObserver.cpp -o orders
orders.exe
```

### Cards
```cmd
g++ CardsDriver.cpp Map.cpp Player.cpp PlayerStrategies.cpp Cards.cpp Orders.cpp GameEngine.cpp CommandProcessing.cpp LoggingObserver.cpp -o cards
cards.exe
```

### Game Engine
```cmd
g++ GameEngineDriver.cpp Map.cpp Player.cpp PlayerStrategies.cpp Cards.cpp Orders.cpp GameEngine.cpp CommandProcessing.cpp LoggingObserver.cpp -o game_engine
game_engine.exe
```

### Command Processing
```cmd
g++ CommandProcessingDriver.cpp Map.cpp Player.cpp PlayerStrategies.cpp Cards.cpp Orders.cpp GameEngine.cpp CommandProcessing.cpp LoggingObserver.cpp -o command_processing
command_processing.exe -file commands.txt
command_processing.exe -console
```

### Logging Observer
```cmd
g++ LoggingObserverDriver.cpp Map.cpp Player.cpp PlayerStrategies.cpp Cards.cpp Orders.cpp GameEngine.cpp CommandProcessing.cpp LoggingObserver.cpp -o logging_observer
logging_observer.exe -file part5commands.txt
logging_observer.exe -console
```

### Player Strategies
```cmd
g++ PlayerStrategiesDriver.cpp Map.cpp Player.cpp PlayerStrategies.cpp Cards.cpp Orders.cpp GameEngine.cpp CommandProcessing.cpp LoggingObserver.cpp -o player_strategies
player_strategies.exe
```
