# apmibConfigFileDecode

For Realtek SDK's config.dat file decode


## Tested target

1. TOTOLINK A3002R
2. DLink DIR-823G
3. And other...

## Usage

```bash
gcc src.c -o decode_apmib_config
./decode_apmib_config INPUT_FILE OUTPUT_FILE
```

example:

```bash
./decode_apmib_config ExportSettings.sh tmp
admin123
```
- The leak string is password for user 'admin'.
