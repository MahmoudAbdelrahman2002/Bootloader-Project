# STM32F446RETx Custom Bootloader

A comprehensive custom bootloader implementation for the STM32F446RETx microcontroller featuring UART-based communication protocol with support for flash programming, memory operations, and device management.

## 🚀 Features

- **Custom Bootloader Protocol**: UART-based communication with host PC
- **Flash Memory Management**: Sector-wise erase, write, and read operations
- **Memory Protection**: Read/Write protection control
- **CRC Verification**: Data integrity checking for reliable communication
- **Multiple Commands**: 12 different bootloader commands for comprehensive device control
- **Address Validation**: Automatic validation for flash and SRAM address ranges
- **OTP Support**: One-Time Programmable memory access

## 📋 Requirements

### Hardware
- **Microcontroller**: STM32F446RETx (Cortex-M4, LQFP64 package)
- **Development Board**: STM32 Nucleo-F446RE or compatible
- **Communication**: UART interface (USART2)
- **External Components**: 
  - User button on PC13 (for bootloader entry)
  - LED on PC7 (status indication)
  - Crystal oscillator (HSE)

### Software
- **IDE**: STM32CubeIDE or compatible ARM GCC toolchain
- **HAL Library**: STM32F4xx HAL Driver
- **Host Software**: Serial terminal or custom host application
- **STM32CubeMX**: For configuration (optional, .ioc file included)

## 🛠️ Building the Project

### Using STM32CubeIDE
1. Import the project into STM32CubeIDE
2. Build the project using Project → Build Project
3. Flash to target using Run → Debug or Run → Run

### Using Command Line (ARM GCC)
```bash
# Ensure ARM GCC toolchain is installed
make clean
make all
```

## 📡 Bootloader Protocol

The bootloader communicates via UART2 (115200 baud, 8N1) using a custom protocol:

### Command Structure
```
[Length] [Command] [Data...] [CRC32]
```

### Supported Commands

| Command Code | Command Name | Description |
|-------------|--------------|-------------|
| `0x51` | BL_GET_VER | Get bootloader version |
| `0x52` | BL_GET_HELP | Get supported command list |
| `0x53` | BL_GET_CID | Get chip identification |
| `0x54` | BL_GET_RDP_STATUS | Get read protection status |
| `0x55` | BL_GOTO_ADDR | Jump to specified address |
| `0x56` | BL_FLASH_ERASE | Erase flash sectors |
| `0x57` | BL_MEM_WRITE | Write data to memory |
| `0x58` | BL_EN_WR_PROTECT | Enable write protection |
| `0x59` | BL_MEM_READ | Read data from memory |
| `0x5A` | BL_READ_SECTOR_STATUS | Read sector protection status |
| `0x5B` | BL_OTP_READ | Read OTP area |
| `0x5C` | BL_DIS_WR_PROTECT | Disable write protection |

### Response Format
```
[ACK/NACK] [Length] [Data...] [CRC32]
```
- **ACK**: `0xA5` - Command accepted
- **NACK**: `0x7F` - Command rejected

## 🔧 Usage Instructions

### 1. Entering Bootloader Mode
- Power on the device while holding the user button (PC13)
- The bootloader will send a welcome message via UART2
- Release the button and start sending commands

### 2. Example Command Sequences

#### Get Bootloader Version
```
Host sends: [0x02] [0x51] [CRC32]
Device responds: [0xA5] [0x01] [0x01] [CRC32]
```

#### Flash Erase (Sector 2)
```
Host sends: [0x03] [0x56] [0x02] [0x01] [CRC32]
Device responds: [0xA5] [0x01] [0x00] [CRC32]  // Success
```

#### Memory Write
```
Host sends: [Length] [0x57] [Address:4] [Data...] [CRC32]
Device responds: [0xA5] [0x01] [Status] [CRC32]
```

### 3. Memory Layout
- **Flash Memory**: `0x08000000` - `0x0807FFFF` (512KB)
- **SRAM**: `0x20000000` - `0x2001FFFF` (128KB)
- **Bootloader**: Sector 0 (`0x08000000` - `0x08007FFF`)
- **Application**: Sector 2+ (`0x08008000` and above)

## 📁 Project Structure

```
Bootloader-Project/
├── Core/
│   ├── Inc/
│   │   ├── bootloader.h          # Bootloader function declarations
│   │   ├── common_macros.h       # Common macro definitions
│   │   ├── main.h                # Main application header
│   │   ├── stm32f4xx_hal_conf.h  # HAL configuration
│   │   └── stm32f4xx_it.h        # Interrupt handlers header
│   ├── Src/
│   │   ├── bootloader.c          # Bootloader implementation
│   │   ├── main.c                # Main application logic
│   │   ├── stm32f4xx_hal_msp.c   # MSP initialization
│   │   ├── stm32f4xx_it.c        # Interrupt handlers
│   │   ├── syscalls.c            # System calls
│   │   ├── sysmem.c              # Memory management
│   │   └── system_stm32f4xx.c    # System initialization
│   └── Startup/
│       └── startup_stm32f446retx.s # Startup assembly code
├── Drivers/
│   ├── CMSIS/                    # CMSIS headers
│   └── STM32F4xx_HAL_Driver/     # HAL library
├── Debug/                        # Debug build outputs
├── Bootloader.ioc               # STM32CubeMX configuration
├── STM32F446RETX_FLASH.ld       # Flash linker script
├── STM32F446RETX_RAM.ld         # RAM linker script
└── README.md                    # This file
```

## 🔍 Key Components

### Bootloader Core (`bootloader.c`)
- Command parsing and execution
- Flash memory operations
- CRC verification
- Address validation
- Memory protection management

### Main Application (`main.c`)
- System initialization
- GPIO configuration for bootloader entry detection
- UART communication setup
- Application jump logic

### Configuration
- **Clock**: HSE crystal with PLL configuration
- **UART2**: 115200 baud, 8N1, connected to ST-Link VCP
- **CRC**: Hardware CRC32 peripheral for data verification
- **GPIO**: PC13 for user button, PC7 for status LED

## 📝 Development Notes

### Flash Memory Protection
The bootloader implements sector-wise protection to prevent accidental modification of the bootloader code itself. Sector 0 and 1 should remain protected in production use.

### Address Validation
All memory operations include address validation to ensure operations are performed within valid memory ranges:
- Flash: `0x08000000` to `0x0807FFFF`
- SRAM: `0x20000000` to `0x2001FFFF`

### CRC Implementation
The bootloader uses STM32's hardware CRC32 peripheral for fast and reliable data verification. All commands include CRC verification for robust communication.

## 🚨 Troubleshooting

### Common Issues

1. **Bootloader not responding**
   - Check UART connection and baud rate (115200)
   - Ensure user button (PC13) is pressed during power-on
   - Verify clock configuration

2. **Flash erase/write failures**
   - Check if target sectors are write-protected
   - Verify address ranges are valid
   - Ensure sufficient power supply

3. **CRC verification failures**
   - Verify host CRC calculation implementation
   - Check for transmission errors
   - Ensure proper byte ordering

### Debug Tips
- Use STM32CubeIDE debugger for step-by-step execution
- Monitor UART traffic with a serial terminal
- Check flash option bytes for protection status
- Verify system clock configuration

## 📄 License

This project is provided under the STMicroelectronics software license terms. See individual source files for specific licensing information.

## 🤝 Contributing

1. Fork the repository
2. Create a feature branch
3. Make your changes
4. Test thoroughly on hardware
5. Submit a pull request

## 📞 Support

For questions or issues related to this bootloader implementation, please:
1. Check the troubleshooting section
2. Review STM32F446 reference manual
3. Consult STM32 HAL documentation
4. Open an issue in this repository

---

**Note**: This bootloader is designed for development and educational purposes. For production use, ensure proper security measures and thorough testing are implemented.