# WORK IN PROGRESS _ NO GUARANTEE THAT IT WORKS ON YOUR PHONE

When you have several LoRa P2P or LoRaWAN nodes with identical firmware a feature to configure the nodes in the field is required. Otherwise you would have to compile the firmware several times with the different configurations.

Many RAK modules use the _**Serial / USB port**_ and _**AT commands**_ to do such a field configuration. But as the RAK4631 has BLE, I wanted to go a different way. Instead of using AT commands, I implemented a custom BLE service and characteristic to receive and send the LoRa/LoRaWAN configuration to an Android phone.

This is still in testing and the example code needs improvement.  
  
## Principle
Instead of hard coding LoRa / LoRaWAN settings in the code, the firmware is using the internal flash to store and read the configurations.  
In a new node, this configuration is set to _**standard**_ values and the node will not automatically try to connect to the LoRa network. After power up of the node, you can connect to the node from an Android application over BLE to setup the node to a different configuration.  

## Requirements  
WisBlock RAK4631    
Android phone or tablet    
Android application [My nRF52 Toolbox](https://play.google.com/store/apps/details?id=tk.giesecke.my_nrf52_tb)    

If you want to compile the Android app by yourself, you can find the source code on [Github](https://github.com/beegee-tokyo/My-nRF52-Toolbox).

## Configurable parameters
### General settings
- Switch between LoRa P2P and LoRaWAN mode
- Enable/disable auto join in LoRaWAN mode

### LoRaWAN settings
- Switch between OTAA and ABP join mode in LoRaWAN mode
- Set network type public or private
- Enable or disable ADR
- Enable or disable confirmed sending
- Enable or disable Dutycycle
- Set Datarate (no check if valid for Region)
- Set TX power (no check if valid for Region)
- Set node class
- Set frequency subband (no check if valid for Region)
- Set fPort for sending
- Set number of join retries
- Set automatic sending repeat time (experimental)

#### LoRaWAN OTAA settings
- Enter Device EUI
- Enter Application EUI
- Enter Application Key

#### LoRaWAN ABP settings
- Enter Device Address
- Enter Network Session Key
- Enter Application Session Key

### LoRa P2P settings
- Select frequency for sending
- Set TX power (no check if valid for Region)
- Set bandwidth
- Set spreading factor
- Set coding rate
- Set preamble length
- set symbol timeout

## Tests
Android application is tested on
- Huawei Mediapad M5 tablet, Android V9
- Samsung Galaxy J2 Core, Android V10
- Vivo V5 (local Philippine brand), Android V8

nRF52 firmware is tested on
- WisBlock Core RAK4631