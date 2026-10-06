# TTG Cardputer-Adv Digital Business Card

Premium offline digital business card firmware for the **M5Stack Cardputer-Adv**.

## Features

- Midnight navy + gold TTG visual identity
- Boot / welcome / menu / about / philosophy / exit screens
- Website QR
- business-card PDF QR for saving Abdul Muhaymin Nawaz's contact
- Company + personal LinkedIn QR codes
- Company + founder Instagram QR codes
- Contact and company information
- Keyboard-first navigation
- Full four-way arrow-key menu navigation
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
Company LinkedIn: https://www.linkedin.com/in/technostics-group  
Personal LinkedIn: https://www.linkedin.com/in/abdul-muhaymin-nawaz-6a3a043b3  
Company Instagram: https://instagram.com/the_technostic  
Founder Instagram: https://instagram.com/jker24256

## Controls

| Key | Action |
|---|---|
| Q | Website QR |
| V | BUSINESS CARD PDF QR |
| C | Contact |
| W | Website |
| L | LinkedIn (company/founder) |
| I | Instagram |
| A | About |
| P | Philosophy |
| E | Exit |
| Enter | Select menu item |
| Up / Left | Previous menu item |
| Down / Right | Next menu item |
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


### Power saving

- Automatically enters ESP32 light sleep after 60 seconds without keyboard interaction.
- The display is put into panel sleep to avoid leaving a static image on the TFT indefinitely.
- Wake the Cardputer-Adv with the top G0/user button.

### Business-card PDF QR

The **V** / business-card QR contains only this direct PDF URL:
`https://technosticsgroup.com/TTG_Business_Card_Founder_CTO.pdf`

The PDF must be hosted at that exact path on the Technostic Group website for the QR to open it.
