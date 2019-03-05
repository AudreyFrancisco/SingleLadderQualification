# basic debugging instructions (still evolving)

## basic checks

- check chip communication (using clock, control)
    ```
    ./test_GRST -c <config>
    ```

- check read-out (using clock, control, data)
    ```
    ./test_digitalscan -c <config>
    ```

## advanced checks

- check write-only communication with chip
(in case of communication problems)
    ```
    ./test_powerdown_analogue -c <config>
    ```
    The analog current should go down below 50 mA.

- check master-slave connections
    ```
    ./test_localbus -c <config>
    ```
