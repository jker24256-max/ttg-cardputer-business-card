#include <Arduino.h>
#include <ctype.h>
#include <Preferences.h>
#include <M5Cardputer.h>
#include <qrcode.h>

// -----------------------------------------------------------------------------
// TTG DIGITAL BUSINESS CARD — M5Stack Cardputer-Adv
// -----------------------------------------------------------------------------

namespace TTG {

// Display
constexpr int W = 240;
constexpr int H = 135;

// Palette
constexpr uint16_t NAVY   = 0x0127;
constexpr uint16_t NAVY2  = 0x020F;
constexpr uint16_t GOLD   = 0xFEA0;
constexpr uint16_t GOLD2  = 0xD68A;
constexpr uint16_t WHITE  = 0xFFFF;
constexpr uint16_t MUTED  = 0xB5B6;
constexpr uint16_t LINE   = 0x3A55;
constexpr uint16_t BLACK  = 0x0000;

// Business identity
constexpr const char* NAME = "Abdul Muhaymin Nawaz";
constexpr const char* TITLE = "Founder & CTO";
constexpr const char* COMPANY = "The Technostic Group";
constexpr const char* TAGLINE = "Praemonitus, Praemunitus";
constexpr const char* WEBSITE = "https://technosticsgroup.com";
constexpr const char* WEBSITE_SHORT = "technosticsgroup.com";
constexpr const char* EMAIL = "abdul@technosticsgroup.com";
constexpr const char* PHONE = "+917439008165";
constexpr const char* LINKEDIN_COMPANY = "https://www.linkedin.com/in/technostics-group";
constexpr const char* LINKEDIN_FOUNDER = "https://www.linkedin.com/in/abdul-muhaymin-nawaz-6a3a043b3";
constexpr const char* IG_COMPANY = "https://instagram.com/the_technostic";
constexpr const char* IG_FOUNDER = "https://instagram.com/jker24256";

// RFC 6350-compatible line endings and explicit field types improve
// compatibility with Android/iOS contact importers.
// The QR formerly contained a vCard. It now contains ONLY the direct PDF URL.
// Host the supplied PDF at this exact path on technosticsgroup.com.
constexpr const char* CARD_PDF_URL =
  "https://technosticsgroup.com/TTG_Business_Card_Founder_CTO.pdf";

enum Screen {
  BOOT, WELCOME, MENU, QR_WEB, QR_VCARD, QR_LINKEDIN_COMPANY, QR_LINKEDIN_FOUNDER,
  QR_IG_COMPANY, QR_IG_FOUNDER, CONTACT, WEBSITE_PAGE,
  LINKEDIN_PAGE, INSTAGRAM_PAGE, ABOUT, PHILOSOPHY, SETTINGS, EXIT
};

Screen screen = BOOT;
int menuIndex = 0;
unsigned long bootStarted = 0;
unsigned long lastActivity = 0;
bool firstFrame = true;
bool displayAsleep = false;
int settingsIndex = 0;
uint8_t brightnessValue = 180;
Preferences prefs;

// After 60 seconds with no interaction, sleep ONLY the TFT.
// The ESP32, keyboard and application remain active so a key wakes the screen.
constexpr unsigned long SLEEP_TIMEOUT_MS = 60000;

// -----------------------------------------------------------------------------
// Power / idle handling
// -----------------------------------------------------------------------------
void render();
void resetIdleTimer();

void applySettings() {
  M5Cardputer.Display.setBrightness(brightnessValue);
}

void saveSettings() {
  prefs.putUChar("brightness", brightnessValue);
}

void wakeDisplayOnly() {
  if (!displayAsleep) return;
  M5Cardputer.Display.wakeup();
  M5Cardputer.Display.setBrightness(brightnessValue);
  displayAsleep = false;
  resetIdleTimer();
  render();
}

void enterDisplaySleep() {
  // Sleep ONLY the TFT. The ESP32 and keyboard remain active.
  M5Cardputer.Display.sleep();
  displayAsleep = true;
}

// -----------------------------------------------------------------------------

void render();

void resetIdleTimer() {
  lastActivity = millis();
}

void wakeDisplay() {
  wakeDisplayOnly();
}

// -----------------------------------------------------------------------------
// Drawing helpers
// -----------------------------------------------------------------------------

void clear() {
  M5Cardputer.Display.fillScreen(NAVY);
}

void header(const char* label) {
  M5Cardputer.Display.fillRect(0, 0, W, 20, NAVY2);
  M5Cardputer.Display.drawFastHLine(0, 19, W, LINE);
  M5Cardputer.Display.setTextColor(GOLD);
  M5Cardputer.Display.setTextDatum(middle_left);
  M5Cardputer.Display.setTextSize(1);
  M5Cardputer.Display.drawString("TTG", 7, 10);
  M5Cardputer.Display.setTextColor(MUTED);
  M5Cardputer.Display.drawString(label, 34, 10);
}

void footer(const char* text = "ESC/X BACK") {
  M5Cardputer.Display.drawFastHLine(0, 122, W, LINE);
  M5Cardputer.Display.setTextDatum(middle_center);
  M5Cardputer.Display.setTextColor(MUTED);
  M5Cardputer.Display.setTextSize(1);
  M5Cardputer.Display.drawString(text, W / 2, 129);
}

// Actual TTG crest, reduced to a 48x64 1-bit bitmap for the Cardputer display.
// Gold pixels are rendered directly on the current screen background.
const uint64_t TTG_CREST[64] PROGMEM = {
  0x000000000000ULL, 0x000000000000ULL, 0x000000000000ULL, 0x000000000000ULL,
  0x000010000000ULL, 0x000018000000ULL, 0x0000DB000000ULL, 0x00007E000000ULL,
  0x00073DE00000ULL, 0x00189BF00000ULL, 0x0030565C0000ULL, 0x004646E60000ULL,
  0x019BC3B38000ULL, 0x0230001DE000ULL, 0x184038067F80ULL, 0x0180E6038080ULL,
  0x0E039180F980ULL, 0x10067CE03B80ULL, 0x0019C3380B00ULL, 0x006301CE0B00ULL,
  0x018C00738B00ULL, 0x0230001CCB00ULL, 0x02400006CB00ULL, 0x024FFFF2CB00ULL,
  0x024FFFF6CB00ULL, 0x02483836CB00ULL, 0x02483816CB00ULL, 0x02403806CB00ULL,
  0x0241B906CB00ULL, 0x0241B986CB00ULL, 0x0241B986CB00ULL, 0x0241B986CB00ULL,
  0x0241B986CBC0ULL, 0x0241B986CBE0ULL, 0x0241B986CB60ULL, 0x0241B986CB40ULL,
  0x0241B986CB00ULL, 0x0241B986C900ULL, 0x0241B986C980ULL, 0x0241B986C980ULL,
  0x0241B986C880ULL, 0x0241B986CAC0ULL, 0x0241B986CAC0ULL, 0x0041B9860B40ULL,
  0x1060B90E0B40ULL, 0x1038383C1AC0ULL, 0x180618F016C0ULL, 0x0C0301C03480ULL,
  0x0600C300E980ULL, 0x01C066078700ULL, 0x10383C3E1E00ULL, 0x1E0E18F0F800ULL,
  0x01C381C7C000ULL, 0x0070C31E0000ULL, 0x00FC667F0000ULL, 0x01032CC18000ULL,
  0x0101B9818000ULL, 0x00009B000000ULL, 0x00029B800000ULL, 0x0003A9C00000ULL,
  0x000125800000ULL, 0x000018000000ULL, 0x000018000000ULL, 0x000010000000ULL
};

void crest(int cx, int cy, int s = 28) {
  const int targetW = s;
  const int targetH = (s * 64) / 48;
  const int x0 = cx - targetW / 2;
  const int y0 = cy - targetH / 2;

  for (int y = 0; y < targetH; ++y) {
    const int srcY = (y * 64) / targetH;
    const uint64_t row = pgm_read_dword(&TTG_CREST[srcY]);
    const uint64_t rowHi = pgm_read_dword(&TTG_CREST[srcY] + 1);
    const uint64_t bits = row | (rowHi << 32);

    for (int x = 0; x < targetW; ++x) {
      const int srcX = (x * 48) / targetW;
      if (bits & (1ULL << srcX)) {
        M5Cardputer.Display.drawPixel(x0 + x, y0 + y, GOLD);
      }
    }
  }
}
void centered(const String& text, int y, uint16_t color = WHITE, int size = 1) {
  M5Cardputer.Display.setTextDatum(middle_center);
  M5Cardputer.Display.setTextColor(color);
  M5Cardputer.Display.setTextSize(size);
  M5Cardputer.Display.drawString(text, W / 2, y);
}

void labelValue(const char* label, const char* value, int y) {
  M5Cardputer.Display.setTextDatum(middle_left);
  M5Cardputer.Display.setTextColor(GOLD2);
  M5Cardputer.Display.drawString(label, 8, y);
  M5Cardputer.Display.setTextColor(WHITE);
  M5Cardputer.Display.drawString(value, 72, y);
}

void wrapText(const String& text, int x, int y, int maxWidth, int lineHeight = 11) {
  M5Cardputer.Display.setTextDatum(top_left);
  M5Cardputer.Display.setTextColor(WHITE);
  M5Cardputer.Display.setTextSize(1);

  String line;
  int yy = y;
  int start = 0;

  while (start < text.length()) {
    int space = text.indexOf(' ', start);
    if (space < 0) space = text.length();
    String word = text.substring(start, space);
    String test = line.length() ? line + " " + word : word;
    if (M5Cardputer.Display.textWidth(test) > maxWidth && line.length()) {
      M5Cardputer.Display.drawString(line, x, yy);
      yy += lineHeight;
      line = word;
    } else {
      line = test;
    }
    start = space + 1;
  }
  if (line.length()) M5Cardputer.Display.drawString(line, x, yy);
}

// -----------------------------------------------------------------------------
// QR
// -----------------------------------------------------------------------------

bool drawQR(const char* payload, int version = 5, uint8_t ecc = ECC_MEDIUM) {
  QRCode qr;
  uint8_t data[qrcode_getBufferSize(version)];

  if (qrcode_initText(&qr, data, version, ecc, payload) != 0) {
    return false;
  }

  // Dedicated scan mode: use the entire display and a large module size.
  // Version 4 is large enough for all TTG URLs used here while allowing
  // 3x3-pixel modules on the 240x135 panel.
  const int module = 3;
  const int quiet = 4 * module;
  const int size = qr.size * module;
  const int total = size + quiet * 2;
  const int ox = (W - size) / 2;
  const int oy = (H - size) / 2;

  M5Cardputer.Display.fillScreen(WHITE);
  M5Cardputer.Display.fillRect(ox - quiet, oy - quiet, total, total, WHITE);

  for (uint8_t y = 0; y < qr.size; ++y) {
    for (uint8_t x = 0; x < qr.size; ++x) {
      if (qrcode_getModule(&qr, x, y)) {
        M5Cardputer.Display.fillRect(
          ox + x * module, oy + y * module, module, module, BLACK);
      }
    }
  }
  return true;
}

// -----------------------------------------------------------------------------
// Screens
// -----------------------------------------------------------------------------

void qrScreen(const char* title, const char* payload, const char* sub) {
  (void)title;
  (void)sub;
  // Lower the backlight in scan mode to prevent camera auto-exposure from
  // washing out the white QR field while keeping the black modules crisp.
  M5Cardputer.Display.setBrightness(min<uint8_t>(brightnessValue, 120));
  drawQR(payload, 5, ECC_MEDIUM);
}

void drawBoot() {
  clear();
  crest(W / 2, 42, 42);
  centered(COMPANY, 77, GOLD, 1);
  centered(TAGLINE, 94, MUTED, 1);
  centered("INITIALIZING", 115, GOLD2, 1);
}

void drawWelcome() {
  clear();
  crest(120, 34, 28);
  centered(COMPANY, 61, GOLD, 1);
  centered(NAME, 77, WHITE, 1);
  centered(TITLE, 91, GOLD2, 1);
  centered(TAGLINE, 108, MUTED, 1);
  footer("ENTER SELECT   X BACK");
}

const char* menuItems[] = {
  "BUSINESS CARD  PDF",
  "CONTACT        DETAILS",
  "WEBSITE        OPEN",
  "LINKEDIN       PROFILE",
  "INSTAGRAM      SOCIAL",
  "ABOUT          COMPANY",
  "PHILOSOPHY     MOTTO",
  "SETTINGS       DISPLAY",
  "EXIT           CLOSE"
};
constexpr int MENU_COUNT = sizeof(menuItems) / sizeof(menuItems[0]);

void drawMenu() {
  clear();
  header("DIGITAL BUSINESS CARD");
  centered(NAME, 28, WHITE, 1);

  for (int i = 0; i < MENU_COUNT; ++i) {
    int y = 42 + i * 9;
    if (i == menuIndex) {
      M5Cardputer.Display.fillRoundRect(5, y - 5, 230, 11, 2, GOLD);
      M5Cardputer.Display.setTextColor(NAVY);
    } else {
      M5Cardputer.Display.setTextColor(MUTED);
    }
    M5Cardputer.Display.setTextDatum(middle_left);
    M5Cardputer.Display.drawString(String(i + 1) + "  " + menuItems[i], 9, y);
  }
  footer("ARROWS MOVE   ENTER SELECT");
}

void drawContact() {
  clear();
  header("CONTACT DETAILS");
  labelValue("NAME", NAME, 34);
  labelValue("ROLE", TITLE, 50);
  labelValue("ORG", COMPANY, 66);
  labelValue("MAIL", EMAIL, 82);
  labelValue("TEL", "+91 7439008165", 98);
  centered(WEBSITE_SHORT, 113, GOLD2, 1);
  footer();
}

void drawWebsite() {
  qrScreen("WEBSITE QR", WEBSITE, WEBSITE_SHORT);
}

void drawLinkedInPage() {
  clear();
  header("LINKEDIN");
  M5Cardputer.Display.drawRoundRect(7, 29, 108, 76, 4, LINE);
  M5Cardputer.Display.drawRoundRect(125, 29, 108, 76, 4, LINE);
  centered("COMPANY", 43, GOLD, 1);
  centered("FOUNDER", 61, WHITE, 1);
  centered("C = COMPANY", 84, MUTED, 1);
  centered("F = FOUNDER", 98, MUTED, 1);
  footer("C COMPANY QR   F FOUNDER QR");
}

void drawInstagramPage() {
  clear();
  header("INSTAGRAM");
  M5Cardputer.Display.drawRoundRect(7, 29, 108, 76, 4, LINE);
  M5Cardputer.Display.drawRoundRect(125, 29, 108, 76, 4, LINE);
  centered("@the_technostic", 43, GOLD, 1);
  centered("@jker24256", 61, WHITE, 1);
  centered("C = COMPANY", 84, MUTED, 1);
  centered("F = FOUNDER", 98, MUTED, 1);
  footer("C COMPANY QR   F FOUNDER QR");
}

void drawAbout() {
  clear();
  header("ABOUT THE GROUP");
  crest(120, 43, 30);
  centered(COMPANY, 69, GOLD, 1);
  wrapText(
    "Technology, security and digital systems built with a disciplined "
    "focus on resilience, privacy and practical engineering.",
    10, 82, 220, 10
  );
  centered(TAGLINE, 111, GOLD2, 1);
  footer();
}

void drawPhilosophy() {
  clear();
  header("PHILOSOPHY");
  centered("PRAEMONITUS", 46, GOLD, 2);
  centered("PRAEMUNITUS", 66, GOLD, 2);
  centered("FOREWARNED", 88, WHITE, 1);
  centered("FOREARMED", 103, WHITE, 1);
  footer();
}

void drawSettings() {
  clear();
  header("SETTINGS");

  const char* labels[] = {"BRIGHTNESS"};
  const int values[] = {brightnessValue};

  for (int i = 0; i < 1; ++i) {
    int y = 58;
    if (i == settingsIndex) {
      M5Cardputer.Display.fillRoundRect(8, y - 14, 224, 29, 4, GOLD);
      M5Cardputer.Display.setTextColor(NAVY);
    } else {
      M5Cardputer.Display.drawRoundRect(8, y - 14, 224, 29, 4, LINE);
      M5Cardputer.Display.setTextColor(MUTED);
    }

    M5Cardputer.Display.setTextDatum(middle_left);
    M5Cardputer.Display.drawString(labels[i], 17, y - 3);

    // Compact level bar.
    int barX = 112;
    int barW = 82;
    int filled = (barW * values[i]) / 255;
    M5Cardputer.Display.drawRect(barX, y - 7, barW, 10, i == settingsIndex ? NAVY : LINE);
    if (filled > 0) {
      M5Cardputer.Display.fillRect(barX + 2, y - 5, max(1, filled - 4), 6,
                                   i == settingsIndex ? NAVY : GOLD);
    }

    M5Cardputer.Display.setTextDatum(middle_right);
    M5Cardputer.Display.drawString(String(values[i]), 220, y - 3);
  }

  footer("LEFT/RIGHT ADJUST   ESC/X BACK");
}

void drawExit() {
  clear();
  crest(120, 42, 34);
  centered("THANK YOU", 75, GOLD, 2);
  centered("CONNECT  |  COLLABORATE", 94, WHITE, 1);
  centered("BUILD  |  SECURE", 107, MUTED, 1);
}

void render() {
  switch (screen) {
    case BOOT: drawBoot(); break;
    case WELCOME: drawWelcome(); break;
    case MENU: drawMenu(); break;
    case QR_WEB: qrScreen("WEBSITE QR", WEBSITE, WEBSITE_SHORT); break;
    case QR_VCARD: qrScreen("BUSINESS CARD PDF", CARD_PDF_URL, "SCAN TO OPEN PDF"); break;
    case QR_LINKEDIN_COMPANY: qrScreen("COMPANY LINKEDIN", LINKEDIN_COMPANY, "The Technostic Group"); break;
    case QR_LINKEDIN_FOUNDER: qrScreen("FOUNDER LINKEDIN", LINKEDIN_FOUNDER, "Abdul Muhaymin Nawaz"); break;
    case QR_IG_COMPANY: qrScreen("COMPANY INSTAGRAM", IG_COMPANY, "@the_technostic"); break;
    case QR_IG_FOUNDER: qrScreen("FOUNDER INSTAGRAM", IG_FOUNDER, "@jker24256"); break;
    case CONTACT: drawContact(); break;
    case WEBSITE_PAGE: drawWebsite(); break;
    case LINKEDIN_PAGE: drawLinkedInPage(); break;
    case INSTAGRAM_PAGE: drawInstagramPage(); break;
    case ABOUT: drawAbout(); break;
    case PHILOSOPHY: drawPhilosophy(); break;
    case SETTINGS: drawSettings(); break;
    case EXIT: drawExit(); break;
  }
  firstFrame = false;
}

void selectMenu() {
  switch (menuIndex) {
    case 0: screen = QR_VCARD; break;
    case 1: screen = CONTACT; break;
    case 2: screen = WEBSITE_PAGE; break;
    case 3: screen = LINKEDIN_PAGE; break;
    case 4: screen = INSTAGRAM_PAGE; break;
    case 5: screen = ABOUT; break;
    case 6: screen = PHILOSOPHY; break;
    case 7: screen = SETTINGS; break;
    case 8: screen = EXIT; break;
  }
}

void back() {
  M5Cardputer.Display.setBrightness(brightnessValue);
  if (screen == MENU || screen == WELCOME) {
    screen = WELCOME;
  } else if (screen == EXIT) {
    screen = MENU;
  } else {
    screen = MENU;
  }
  render();
}

void directKey(char key) {
  switch (tolower((unsigned char)key)) {
    case 'q': screen = QR_WEB; break;
    case 'v': screen = QR_VCARD; break;
    case 'c': screen = CONTACT; break;
    case 'w': screen = WEBSITE_PAGE; break;
    case 'l': screen = LINKEDIN_PAGE; break;
    case 'i': screen = INSTAGRAM_PAGE; break;
    case 'a': screen = ABOUT; break;
    case 'p': screen = PHILOSOPHY; break;
    case 's': screen = SETTINGS; break;
    case 'e': screen = EXIT; break;
    case 'x': back(); return;
    default: return;
  }
  render();
}

void handleKeys() {
  if (!M5Cardputer.Keyboard.isChange()) return;
  if (!M5Cardputer.Keyboard.isPressed()) return;

  if (displayAsleep) {
    wakeDisplayOnly();
    return;
  }

  resetIdleTimer();
  auto st = M5Cardputer.Keyboard.keysState();

  if (st.esc) {
    back();
    return;
  }

  if (screen == WELCOME && st.enter) {
    screen = MENU;
    render();
    return;
  }

  if (screen == SETTINGS) {
    if (st.left || st.right) {
      int delta = st.right ? 16 : -16;
      brightnessValue = constrain((int)brightnessValue + delta, 0, 255);
      applySettings();
      saveSettings();
      render();
      return;
    }
  }

  if (screen == MENU) {
    // All four physical arrow keys navigate the menu.
    // Up/Left = previous item, Down/Right = next item.
    if (st.up || st.left) {
      menuIndex = (menuIndex + MENU_COUNT - 1) % MENU_COUNT;
      render();
      return;
    }
    if (st.down || st.right) {
      menuIndex = (menuIndex + 1) % MENU_COUNT;
      render();
      return;
    }
    if (st.enter) {
      selectMenu();
      render();
      return;
    }
  }

  for (char c : st.word) {
    if (c == 'x' || c == 'X') {
      back();
      return;
    }

    if (screen == LINKEDIN_PAGE) {
      if (c == 'c' || c == 'C') {
        screen = QR_LINKEDIN_COMPANY;
        render();
        return;
      }
      if (c == 'f' || c == 'F') {
        screen = QR_LINKEDIN_FOUNDER;
        render();
        return;
      }
    }

    if (screen == INSTAGRAM_PAGE) {
      if (c == 'c' || c == 'C') {
        screen = QR_IG_COMPANY;
        render();
        return;
      }
      if (c == 'f' || c == 'F') {
        screen = QR_IG_FOUNDER;
        render();
        return;
      }
    }

    directKey(c);
    return;
  }
}

} // namespace TTG

void setup() {
  auto cfg = M5.config();
  M5Cardputer.begin(cfg, true);
  M5Cardputer.Display.setRotation(1);
  M5Cardputer.Display.setTextFont(1);
  M5Cardputer.Display.setTextSize(1);

  TTG::prefs.begin("ttg", false);
  TTG::brightnessValue = TTG::prefs.getUChar("brightness", 180);
  TTG::applySettings();

  TTG::bootStarted = millis();
  TTG::lastActivity = millis();
  TTG::render();
  TTG::resetIdleTimer();
}

void loop() {
  M5Cardputer.update();

  if (TTG::screen == TTG::BOOT) {
    if (millis() - TTG::bootStarted > 1800) {
      TTG::screen = TTG::WELCOME;
      TTG::resetIdleTimer();
      TTG::render();
    }
  } else {
    TTG::handleKeys();

    if (!TTG::displayAsleep &&
        millis() - TTG::lastActivity >= TTG::SLEEP_TIMEOUT_MS) {
      TTG::enterDisplaySleep();
    }
  }

  delay(5);
}
