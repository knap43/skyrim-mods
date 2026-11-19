# Starting Perk Points

Gives your character a configurable number of extra perk points, exactly once:

- **New game** – the points are granted as soon as the game starts.
- **Existing save** – the points are granted the first time you load a save with the mod
  installed, so it is safe to add mid-playthrough.

The grant is recorded in the save's SKSE co-save (`.skse`), so loading the same character
again never grants the points twice. Changing the INI later does not affect characters that
have already received their points.

No ESP is needed. The mod uses no plugin slot.

## Configuration

`Data/SKSE/Plugins/StartingPerkPoints.ini`:

```ini
[General]
iPerkPoints = 3
```

The range is 0–127, because the game stores unspent perk points in a signed byte.

## Requirements

- Skyrim SE 1.5.97
- SKSE64 2.0.20
- Address Library for SKSE Plugins

## Troubleshooting

The plugin writes a log to
`Documents/My Games/Skyrim Special Edition/SKSE/StartingPerkPoints.log`.
