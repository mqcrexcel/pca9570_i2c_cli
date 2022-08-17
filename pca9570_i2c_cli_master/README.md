Control LED using PCA9570 i2C and CLI via UART

The command structure should be as follows:
"[pattern] [color] [parameter(optional)]"

Where:
[pattern] is either "blink", "shine" or "off" 
[color] is either "green", "red" or "blue"
[parameter] is:
blinking interval in ms for "blink" pattern

The device should also respond to the following commands:
"cancel" which will set all LEDs off
"help" which will print help text (it can be this document itself)
"demo" which will cycle through different modes in whichever way/order

Any incorrect command should cancel the current one 