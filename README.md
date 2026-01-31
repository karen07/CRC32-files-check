# CRC32 files check

CRC32 files check is a small file-content monitor that records CRC32 checksums for regular files present in a directory at startup and periodically checks whether those files have changed.

The program is intentionally simple: it opens the initial set of regular files, keeps their current checksums in memory, and reports later checksum changes to stdout and syslog. The directory and polling interval can be supplied either through command-line arguments or environment variables.

CRC32 is used here as a fast change detector, not as a cryptographic integrity mechanism. The program does not provide authenticated integrity and is not intended to detect every possible directory-level change such as newly added files.

## Описание

CRC32 files check - небольшой монитор содержимого файлов. При запуске он запоминает CRC32 для обычных файлов, уже находящихся в указанном каталоге, и затем периодически проверяет, изменилось ли содержимое этих файлов.

Программа намеренно сделана простой: она открывает исходный набор файлов, хранит их текущие контрольные суммы в памяти и сообщает о последующих изменениях через stdout и syslog. Каталог и интервал проверки можно задать аргументами командной строки или переменными окружения.

CRC32 используется здесь как быстрый способ обнаружения изменений, а не как криптографический механизм контроля целостности. Программа не обеспечивает аутентифицированную проверку и не предназначена для обнаружения всех изменений самого каталога, например появления новых файлов.

## Сборка

```sh
cmake --preset release
cmake --build --preset release
```

Исполняемый файл:

```text
build/release/CRC32-files-check
```

## Запуск

```text
Commands:
  Required parameters:
    -f  "/test/"  Directory to check
    -t  "60"      Check interval in seconds
```

Пример:

```sh
./build/release/CRC32-files-check -f /srv/data -t 60
```

Те же параметры можно задать через окружение:

```sh
CHECK_FOLDER=/srv/data CHECK_FOLDER_TIME=60 ./build/release/CRC32-files-check
```

Параметры командной строки переопределяют значения из окружения.

## Как работает

При старте программа проходит по обычным файлам в указанном каталоге и считает для них CRC32. Затем через заданный интервал контрольные суммы считаются повторно и сравниваются с исходными значениями.

Проект предназначен прежде всего для простого мониторинга изменений файлов, когда важны минимальная зависимость от внешних библиотек и небольшой объем кода.
