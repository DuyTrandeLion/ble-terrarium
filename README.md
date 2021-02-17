# Description

This device is built for those who enjoy riding motorbike, especially in Vietnam, Thailand, China... It works as a GPS tracker and media controller over BLE. The user's coordinate is transmitted to the phone or can be upload to a specific server (under development). User can also use this device as a media controller (play/pause/stop music, accept a phone call).

## Installation

```bash
git clone https://github.com/DuyTrandeLion/ble-gnss.git
```

Open the project with [Segger Embedded Studio](https://www.segger.com/products/development-tools/embedded-studio/), compile and program the device

## Usage

To calibrate the e-compass on the device, do the following steps:
1. Place the device on a steady surface and wait around 6 seconds to calibrate the Gyroscope.
2. Move the device with an increasing angle of 45 degrees to calibrate to Accelerometer.
3. Draw any figure in the air with the device to calibrate the Magnetometer.

## Contributing
Pull requests are welcome. For major changes, please open an issue first to discuss what you would like to change.

Please make sure to update tests as appropriate.

## License
[MIT](https://choosealicense.com/licenses/mit/)