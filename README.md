# BNT Candles Desktop

Qt 6 and C++ business-management app with customer, product, order, expense and report screens backed by MySQL.

## Build

Install a C++17 toolchain, CMake, Qt 6 Core/Gui/Sql/Widgets/Charts, the Qt MySQL driver and MySQL client headers/libraries. Configure paths for your machine:

```sh
cmake -S . -B build -DCMAKE_PREFIX_PATH=/path/to/Qt -DMYSQL_INCLUDE_DIR=/path/to/mysql/include -DMYSQL_LIBRARY=/path/to/mysql/library
cmake --build build
```

Use a Qt kit and MySQL library compatible with the selected compiler. The existing Windows MySQL search hints are retained; there is no hard-coded Qt installation path.

## Database configuration

Copy `assets/db_cred.example.ini` to a file outside the repository, fill in your database details and set `BNT_DB_CONFIG` to its absolute path. Without that variable, the app looks for `db_cred.ini` in Qt's application configuration directory. The existing `test_db` settings group is retained for compatibility.

Credentials are not bundled into Qt resources. This repository does not include the database schema; restore a compatible database before using the screens.

## Layout

- `cpp/` and `headers/`: application and screen code.
- `ui/`: Qt Designer forms.
- `assets/`: images, styles and lookup data.

## Review changes

Corrects expense-window filename casing, generated UI include paths and the root main-window include. Adds an AUTOUIC search path and external database configuration. Full Qt/MySQL compilation and desktop workflows still need validation on a configured machine.
## Related projects

See the [project index](https://github.com/Jaeger790#projects) for the rest of my work.
