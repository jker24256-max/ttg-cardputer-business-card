# TTG Cardputer-Adv Digital Business Card

Premium offline digital business card firmware for the **M5Stack Cardputer-Adv**.

## Features

- Midnight navy + gold TTG visual identity
- Boot / welcome / menu / about / philosophy / exit screens
- Website QR
- vCard QR for saving Abdul Muhaymin Nawaz's contact
- LinkedIn QR
- Company + founder Instagram QR codes
- Contact and company information
- Keyboard-first navigation
- Works offline; Wi-Fi is not required for the card itself
- Centralized business identity constants
- PlatformIO build configuration

## Identity

**Abdul Muhaymin Nawaz**  
Founder & CTO  
**The Technostic Group**  
Praemonitus, Praemunitus

Website: https://technosticsgroup.com  
Email: abdul@technosticsgroup.com  
Phone: +91 7439008165  
LinkedIn: https://www.linkedin.com/in/technostics-group  
Company Instagram: https://instagram.com/the_technostic  
Founder Instagram: https://instagram.com/jker24256

## Controls

| Key | Action |
|---|---|
| Q | Website QR |
| V | vCard QR |
| C | Contact |
| W | Website |
| L | LinkedIn |
| I | Instagram |
| A | About |
| P | Philosophy |
| E | Exit |
| Enter | Select menu item |
| Up / Down | Move |
| Esc / X | Back |

## Build

Install PlatformIO, then:

```bash
pio run
pio run --target upload
pio device monitor
```

For the first upload, put the Cardputer-Adv into USB download mode if required by your setup.

## Dependencies

- M5Stack M5Cardputer
- ricmoo/QRCode

The QR generator is used as an external dependency rather than copied into this repository.
