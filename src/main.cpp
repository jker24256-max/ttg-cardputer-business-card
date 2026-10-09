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
  LINKEDIN_PAGE, INSTAGRAM_PAGE, ABOUT, PHILOSOPHY, SETTINGS, EXIT, EASTER_EGG
};

Screen screen = BOOT;
int menuIndex = 0;
unsigned long bootStarted = 0;
unsigned long lastActivity = 0;
bool firstFrame = true;
bool displayAsleep = false;
int settingsIndex = 0;
// 230 is the practical default for the Cardputer-Adv display.
// Users can still fine-tune it from Settings.
uint8_t brightnessValue = 230;
String secretBuffer;
Preferences prefs;
unsigned long lastBootFrame = 0;
int bootLastReveal = -1;
int bootLastProgress = -1;

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

void technicalBackground() {
  M5Cardputer.Display.fillScreen(NAVY);
  M5Cardputer.Display.drawFastHLine(0, 26, W, 0x0A1D);
  M5Cardputer.Display.drawFastHLine(0, 120, W, 0x0A1D);

  // Fine circuit traces — deliberately subtle.
  const int ys[] = {31, 116};
  for (int i = 0; i < 2; ++i) {
    M5Cardputer.Display.drawFastHLine(12, ys[i], 48, 0x1830);
    M5Cardputer.Display.drawFastHLine(60, ys[i], 22, 0x1830);
    M5Cardputer.Display.drawFastVLine(60, i == 0 ? ys[i] : ys[i] - 12, 12, 0x1830);
    M5Cardputer.Display.fillCircle(60, ys[i], 1, GOLD2);
  }
  M5Cardputer.Display.drawFastHLine(183, 31, 38, 0x1830);
  M5Cardputer.Display.drawFastVLine(183, 31, 13, 0x1830);
  M5Cardputer.Display.fillCircle(183, 44, 1, GOLD2);
}

void clear() {
  technicalBackground();
}

int pageNumber() {
  switch (screen) {
    case MENU: return 1;
    case CONTACT: return 2;
    case WEBSITE_PAGE: return 3;
    case LINKEDIN_PAGE: return 4;
    case INSTAGRAM_PAGE: return 5;
    case ABOUT: return 6;
    case PHILOSOPHY: return 7;
    case SETTINGS: return 8;
    case EXIT: return 9;
    default: return 0;
  }
}

void drawBatteryStatus() {
  int level = M5Cardputer.Power.getBatteryLevel();

  // Compact battery glyph only; keep the 240px header uncluttered.
  M5Cardputer.Display.drawRect(W - 23, 6, 16, 8, MUTED);
  M5Cardputer.Display.fillRect(W - 5, 8, 2, 4, MUTED);

  if (level >= 0) {
    const int fill = constrain((14 * level) / 100, 1, 14);
    M5Cardputer.Display.fillRect(
      W - 21, 8, fill, 4, level <= 25 ? GOLD2 : GOLD);
  }
}

void header(const char* label) {
  // Clean three-zone header:
  // TTG identity | section title | page + battery.
  M5Cardputer.Display.fillRect(0, 0, W, 20, NAVY2);
  M5Cardputer.Display.drawFastHLine(0, 19, W, LINE);

  M5Cardputer.Display.setTextSize(1);
  M5Cardputer.Display.setTextDatum(middle_left);

  M5Cardputer.Display.setTextColor(GOLD);
  M5Cardputer.Display.drawString("TTG", 7, 10);

  M5Cardputer.Display.setTextColor(MUTED);
  M5Cardputer.Display.drawString(label, 29, 10);

  const int p = pageNumber();
  if (p > 0) {
    M5Cardputer.Display.setTextDatum(middle_right);
    M5Cardputer.Display.setTextColor(GOLD2);
    M5Cardputer.Display.drawString(String(p) + "/09", 204, 10);
  }

  drawBatteryStatus();
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
  0x000000000000ULL,
  0x000000000000ULL,
  0x000000000000ULL,
  0x000001800000ULL,
  0x000001800000ULL,
  0x00000FF00000ULL,
  0x000007E00000ULL,
  0x00007BFE0000ULL,
  0x0001CDFF8000ULL,
  0x000327ECC000ULL,
  0x0006FC7F7000ULL,
  0x00199C3BBC00ULL,
  0x07F30181EFF0ULL,
  0x0FCE07E073F8ULL,
  0x18381C383E18ULL,
  0x0DE073DE0FB8ULL,
  0x0581CE7781B0ULL,
  0x0587381DE1B0ULL,
  0x059CE00F79B0ULL,
  0x05938003DDB0ULL,
  0x05960000EDB0ULL,
  0x0596FFFF6DB0ULL,
  0x0596FFFF6DB0ULL,
  0x0596C3C36DB0ULL,
  0x0596C3C16DB0ULL,
  0x059603C06DB0ULL,
  0x05960BD86DB0ULL,
  0x05960BD86DB0ULL,
  0x05960BD86DB0ULL,
  0x05960BD86DB0ULL,
  0x3D960BD86DBCULL,
  0x2D960BD86DBCULL,
  0x25960BD86DBCULL,
  0x05960BD86DBCULL,
  0x05960BD86DB0ULL,
  0x0D960BD86DB0ULL,
  0x09960BD86DB0ULL,
  0x09960BD86D98ULL,
  0x19960BD86DF8ULL,
  0x15960BD86DB8ULL,
  0x15960BD869ACULL,
  0x15870BD8E1ACULL,
  0x1683C1C3C1ECULL,
  0x12C0718F0378ULL,
  0x1B60381C06D8ULL,
  0x0DB80E301FB4ULL,
  0x061F0760F9F8ULL,
  0x03C3C3C3E7D0ULL,
  0x00F8718F3F20ULL,
  0x000F1C38F8C0ULL,
  0x0007CE73E700ULL,
  0x000FF26FF800ULL,
  0x00183BDC1800ULL,
  0x000809981800ULL,
  0x00001DB80000ULL,
  0x00003DBC0000ULL,
  0x00003BDC0000ULL,
  0x000003C00000ULL,
  0x000001800000ULL,
  0x000001800000ULL,
  0x000000000000ULL,
  0x000000000000ULL,
  0x000000000000ULL,
  0x000000000000ULL
};

void crest(int cx, int cy, int s = 28) {
  const int targetW = s;
  const int targetH = (s * 64) / 48;
  const int x0 = cx - targetW / 2;
  const int y0 = cy - targetH / 2;

  for (int y = 0; y < targetH; ++y) {
    const int srcY = (y * 64) / targetH;
    const uint32_t low = pgm_read_dword(reinterpret_cast<const uint32_t*>(&TTG_CREST[srcY]));
    const uint32_t high = pgm_read_dword(reinterpret_cast<const uint32_t*>(&TTG_CREST[srcY]) + 1);
    const uint64_t bits = static_cast<uint64_t>(low) |
                          (static_cast<uint64_t>(high) << 32);

    for (int x = 0; x < targetW; ++x) {
      const int srcX = (x * 48) / targetW;
      if (bits & (1ULL << srcX)) {
        M5Cardputer.Display.drawPixel(x0 + x, y0 + y, GOLD);
      }
    }
  }
}
void centeredAt(const String& text, int x, int y, uint16_t color = WHITE, float size = 1.0f) {
  M5Cardputer.Display.setTextDatum(middle_center);
  M5Cardputer.Display.setTextColor(color);
  M5Cardputer.Display.setTextSize(size);
  M5Cardputer.Display.drawString(text, x, y);
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

bool drawQR(const char* payload, int preferredVersion = 4, uint8_t ecc = ECC_LOW) {
  QRCode qr;
  // Keep one fixed buffer large enough for our supported scan versions.
  // This lets us try a smaller QR first and fall back if the payload needs it.
  constexpr int MAX_QR_VERSION = 5;
  uint8_t data[qrcode_getBufferSize(MAX_QR_VERSION)];

  int selectedVersion = 0;

  // Prefer the smallest practical symbol. Smaller versions mean fewer
  // modules, which gives the Cardputer camera target more usable detail.
  for (int version = 3; version <= MAX_QR_VERSION; ++version) {
    if (version > preferredVersion && selectedVersion != 0) break;
    if (qrcode_initText(&qr, data, version, ecc, payload) == 0) {
      selectedVersion = version;
      break;
    }
  }

  if (selectedVersion == 0) {
    return false;
  }

  // The display is only 135px tall, so version 4/5 symbols are limited to
  // 3px modules. Shorter payloads fit version 3 and get 4px modules.
  int module = (qr.size <= 29) ? 4 : 3;
  int quiet = 4 * module;

  // A version-3 code at 4px/module would exceed the panel with the full
  // standard quiet zone. The surrounding display is already pure white, so
  // use a compact 2-module internal margin in that case; the screen itself
  // continues the white quiet area beyond it.
  if (qr.size <= 29) {
    quiet = 2 * module;
  }

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
  // QR scan mode needs maximum optical contrast at normal phone distance.
  // Restore the user's normal brightness when leaving the QR screen.
  M5Cardputer.Display.setBrightness(255);
  drawQR(payload, 4, ECC_LOW);
}

void drawBoot() {
  const unsigned long elapsed = millis() - bootStarted;
  const int reveal = min(42, 8 + (int)(elapsed / 55));
  const int progress = min(120, (int)((elapsed * 120UL) / 1800UL));

  // Draw the static boot frame only once. Subsequent frames update only
  // the animated regions, avoiding the full-screen redraw/flicker.
  if (bootLastReveal < 0) {
    clear();
    centered(COMPANY, 77, GOLD, 1);
    centered(TAGLINE, 94, MUTED, 1);
    M5Cardputer.Display.drawRect(60, 122, 120, 4, LINE);
    bootLastReveal = 0;
    bootLastProgress = 0;
  }

  // Erase only the crest area, then redraw it at the new size.
  if (reveal != bootLastReveal) {
    M5Cardputer.Display.fillRect(91, 8, 58, 69, NAVY);
    crest(W / 2, 42, reveal);
    bootLastReveal = reveal;
  }

  // Update only the status text when it changes.
  static int lastStatus = -1;
  int status = elapsed < 450 ? 0 :
               elapsed < 900 ? 1 :
               elapsed < 1300 ? 2 : 3;
  if (status != lastStatus) {
    M5Cardputer.Display.fillRect(55, 101, 130, 15, NAVY);
    const char* text = status == 0 ? "LOADING PROFILE" :
                       status == 1 ? "VERIFYING IDENTITY" :
                       status == 2 ? "SYSTEM READY" : "WELCOME";
    centered(text, 112, GOLD2, 1);
    lastStatus = status;
  }

  if (progress != bootLastProgress) {
    if (progress > bootLastProgress) {
      M5Cardputer.Display.fillRect(60 + bootLastProgress, 122,
                                   progress - bootLastProgress, 4, GOLD);
    } else {
      M5Cardputer.Display.fillRect(60, 122, 120, 4, LINE);
      M5Cardputer.Display.fillRect(60, 122, progress, 4, GOLD);
    }
    bootLastProgress = progress;
  }
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
      M5Cardputer.Display.setTextColor(GOLD);
      M5Cardputer.Display.drawString(">", 5, y);
      M5Cardputer.Display.drawFastHLine(17, y + 5, 208, GOLD2);
    } else {
      M5Cardputer.Display.setTextColor(MUTED);
    }
    M5Cardputer.Display.setTextDatum(middle_left);
    M5Cardputer.Display.drawString(String(i + 1) + "  " + menuItems[i], 9, y);
  }
  footer("ARROWS MOVE   ENTER SELECT");
}

void contactIcon(int x, int y, char type) {
  M5Cardputer.Display.setTextDatum(middle_center);
  M5Cardputer.Display.setTextColor(GOLD2);
  M5Cardputer.Display.setTextSize(1);

  if (type == 'P') {
    M5Cardputer.Display.drawRoundRect(x - 5, y - 7, 10, 14, 2, GOLD2);
    M5Cardputer.Display.drawFastHLine(x - 2, y + 4, 4, GOLD2);
  } else if (type == 'M') {
    M5Cardputer.Display.drawRect(x - 7, y - 5, 14, 10, GOLD2);
    M5Cardputer.Display.drawFastHLine(x - 6, y - 4, 5, GOLD2);
    M5Cardputer.Display.drawFastHLine(x + 6, y - 4, -5, GOLD2);
  } else {
    M5Cardputer.Display.drawCircle(x, y, 6, GOLD2);
    M5Cardputer.Display.drawFastHLine(x - 3, y, 6, GOLD2);
  }
}

void contactRightText(const char* text, int y, uint16_t color, float size = 1.0f) {
  M5Cardputer.Display.setTextDatum(middle_right);
  M5Cardputer.Display.setTextColor(color);
  M5Cardputer.Display.setTextSize(size);
  M5Cardputer.Display.drawString(text, 222, y);
}

void drawContact() {
  clear();
  header("CONTACT");

  // Treat this as a miniature premium credential card rather than a
  // compressed copy of the physical business card.
  M5Cardputer.Display.fillRoundRect(8, 30, 224, 82, 5, NAVY2);
  M5Cardputer.Display.drawRoundRect(8, 30, 224, 82, 5, GOLD2);
  M5Cardputer.Display.drawFastVLine(69, 38, 66, LINE);
  crest(39, 61, 29);

  M5Cardputer.Display.setTextDatum(middle_left);
  M5Cardputer.Display.setTextColor(GOLD);
  M5Cardputer.Display.setTextSize(1.10f);
  M5Cardputer.Display.drawString(NAME, 78, 43);

  M5Cardputer.Display.setTextColor(GOLD2);
  M5Cardputer.Display.setTextSize(1.0f);
  M5Cardputer.Display.drawString(TITLE, 78, 55);
  M5Cardputer.Display.drawFastHLine(78, 63, 140, LINE);

  // No contact icons: the freed horizontal space is dedicated to
  // larger, cleaner typography for the actual contact information.
  M5Cardputer.Display.setTextDatum(middle_left);
  M5Cardputer.Display.setTextColor(WHITE);
  M5Cardputer.Display.setTextSize(1.05f);
  M5Cardputer.Display.drawString("+91 7439008165", 78, 74);

  // Avoid fractional text scaling for the email: split it into two
  // readable lines using the native font. The domain also serves as the
  // website address, so it does not need to be printed a second time.
  M5Cardputer.Display.setTextSize(1);
  M5Cardputer.Display.drawString("abdul@", 78, 85);
  M5Cardputer.Display.drawString("technosticsgroup.com", 78, 98);

  footer("ESC/X BACK");
}

void drawWebsite() {
  qrScreen("WEBSITE QR", WEBSITE, WEBSITE_SHORT);
}

void linkedInIcon(int cx, int cy) {
  M5Cardputer.Display.drawRoundRect(cx - 18, cy - 18, 36, 36, 4, GOLD);
  M5Cardputer.Display.setTextDatum(middle_center);
  M5Cardputer.Display.setTextColor(GOLD);
  M5Cardputer.Display.drawString("in", cx, cy);
}

void instagramIcon(int cx, int cy) {
  M5Cardputer.Display.drawRoundRect(cx - 18, cy - 18, 36, 36, 8, GOLD);
  M5Cardputer.Display.drawCircle(cx, cy, 8, GOLD);
  M5Cardputer.Display.fillCircle(cx + 11, cy - 11, 2, GOLD);
}

// Matched social-profile selector. Keep both screens deliberately
// identical in geometry: the icon identifies the service, the label
// identifies the destination owner, and the footer provides the action.
void drawSocialSelector(const char* section, bool instagram) {
  clear();
  header(section);

  // Two balanced cards with generous breathing room for the 240x135 panel.
  M5Cardputer.Display.drawRoundRect(8, 29, 106, 78, 6, LINE);
  M5Cardputer.Display.drawRoundRect(126, 29, 106, 78, 6, LINE);

  if (instagram) {
    instagramIcon(61, 53);
    instagramIcon(179, 53);
  } else {
    linkedInIcon(61, 53);
    linkedInIcon(179, 53);
  }

  // Keep destination details off the selector. The QR screen is the
  // destination surface and carries the actual profile URL.
  centeredAt("COMPANY", 61, 81, GOLD, 1.0f);
  centeredAt("FOUNDER", 179, 81, WHITE, 1.0f);

  // Interaction keys are visually subordinate to the identity labels.
  centeredAt("C", 61, 97, GOLD2, 1.0f);
  centeredAt("F", 179, 97, GOLD2, 1.0f);

  footer("C / F   SELECT PROFILE");
}

void drawLinkedInPage() {
  drawSocialSelector("LINKEDIN", false);
}

void drawInstagramPage() {
  drawSocialSelector("INSTAGRAM", true);
}

void drawAbout() {
  clear();
  header("ABOUT");

  crest(24, 43, 20);
  centered(COMPANY, 68, GOLD, 1);

  // Tighter, deliberate text block so it never collides with the footer.
  centered("Technology, security and digital", 82, WHITE, 1);
  centered("systems built with a disciplined", 92, WHITE, 1);
  centered("focus on resilience, privacy and", 102, WHITE, 1);
  centered("practical engineering.", 112, WHITE, 1);

  footer("ESC/X BACK");
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

void drawEasterEgg() {
  clear();
  header("TTG // CLASSIFIED");
  crest(120, 48, 42);
  centered("PRAEMONITUS", 80, GOLD, 1);
  centered("PRAEMUNITUS", 94, WHITE, 1);
  centered("EST. 2025  //  SYSTEM NOMINAL", 109, MUTED, 1);
  footer("ESC/X BACK");
}

void drawExit() {
  clear();
  crest(120, 42, 34);
  centered("THANK YOU", 75, GOLD, 2);
  centered("CONNECT  |  COLLABORATE", 94, WHITE, 1);
  centered("BUILD  |  SECURE", 107, MUTED, 1);
}

void drawCurrentScreen() {
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
    case EASTER_EGG: drawEasterEgg(); break;
  }
}

void render() {
  drawCurrentScreen();
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
  } else if (screen == EXIT || screen == EASTER_EGG) {
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

  // Make Settings escapable through both the dedicated ESC key and Enter,
  // in addition to X (handled globally below). This gives a reliable
  // keyboard exit even when the user is adjusting brightness.
  if (screen == SETTINGS && st.enter) {
    back();
    return;
  }

  // Handle X before screen-specific navigation so it always exits a page.
  for (char c : st.word) {
    if (c == 'x' || c == 'X') {
      back();
      return;
    }
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

    if (isalpha((unsigned char)c)) {
      secretBuffer += (char)tolower((unsigned char)c);
      if (secretBuffer.length() > 3) {
        secretBuffer.remove(0, secretBuffer.length() - 3);
      }
      if (secretBuffer == "ttg") {
        screen = EASTER_EGG;
        secretBuffer = "";
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

  // One-time migration for older firmware whose default was 180.
  // Existing users get the brighter Cardputer profile once, while future
  // manual brightness changes remain persistent.
  const bool brightnessV2 = TTG::prefs.getBool("brightness_v2", false);
  if (!brightnessV2) {
    TTG::brightnessValue = 230;
    TTG::prefs.putUChar("brightness", TTG::brightnessValue);
    TTG::prefs.putBool("brightness_v2", true);
  } else {
    TTG::brightnessValue = TTG::prefs.getUChar("brightness", 230);
  }
  TTG::applySettings();

  TTG::bootStarted = millis();
  TTG::bootLastReveal = -1;
  TTG::bootLastProgress = -1;
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
    } else if (millis() - TTG::lastBootFrame > 70) {
      TTG::lastBootFrame = millis();
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
