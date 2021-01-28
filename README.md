# Minecraft Dungeon Finder

## Basic Intro

This is a multi-threaded command-line C program that scan minecraft savegames and find locations and spawning type of dungeons.

It only searches in **existing** chunks, so dungeons in chunks that no players have visited will not be found.

Thanks to @chmod222, who wrote the marvelous cNBT library that parses raw NBT data for me! I will buy them some beer if I met them some day.

Currently this tool has only been tested on Minecraft 1.13.2, 1.13 and 1.11.2 savegames. I will test on more versions as I expand functionality of this tool. (Issues welcomed! <3)

## Usage

`./dungeon_finder [-t num_threads] [-i includeid1,id2,... | -e exclude_id1,id2,...] <region directory|.mca file>`

`-t` specifies the number of threads that will be used. It defaults to the number of logical processors.


`-i` specifies the only ids that will be shown in the result. e.g -i minecraft:skeleton will only search for skeletons.

`-e` specifieds the ids that will be excluded from the results.

The ids following `-i` and `-e` should be comma separated, with no spaces in and between ids.

The path to minecraft savegame region folder or .mca file should be the last option.

### Example Usage:

use 3 threads to search for skeleton spawners in the nether

```
./dungeon_finder -t 3 -i minecraft:skeleton /home/mc/savedata1/world/DIM-1/
```

Search for all spawners that are not spider spawners in the overworld

```
./dungeon_finder -e minecraft:cave_spider,minecraft:spider /home/mc/savedata1/world/region/
```

## Why C?

This is a project for personal practice, so I chose the language that I had no individual project experience with.

For additional practice purposes, I wrote my own versions of thread-safe hashtable, vector and queue.

The repository can be found at [https://github.com/AlexHalogen/C_data_structures](https://github.com/AlexHalogen/C_data_structures)

## TODOS

- [ ] Improve command line arguments parsing
- [ ] Revise the part of matching NBT tag names
- [ ] Port to Windows
- [ ] Minecraft 1.14+ support
