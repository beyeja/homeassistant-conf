using namespace esphome;
using namespace display;

const bool DEBUG_LAYOUT = false;

void layoutLines(display::Display &it, int sX, int sY, int w, int h) {
  int gWidth = 50;

  if (!DEBUG_LAYOUT)
    return;

  // vertical
  for (int i = sX; i <= w; i += gWidth) {
    // fromX, fromY, toX, toY
    it.line(i, sY, i, h + sY);
  }
  it.line(w, sY, w, h + sY);

  // horizontal
  for (int i = sY; i <= h; i += gWidth) {
    // fromX, fromY, toX, toY
    it.line(sX, i, w, i);
  }
  it.line(sX, h + sY, w, h + sY);
}

// map forecast to icon
esphome::image::Image *forecastIcon(std::__cxx11::string forecastVal, bool xl) {
  if (forecastVal == "sunny" 
  || forecastVal == "clear-day") {
    return &id(xl ? icnSunnyXL : icnSunnyMD);
  } else if (forecastVal == "clear-night") {
    return &id(xl ? icnClearNightXL : icnClearNightMD);
  } else if (forecastVal == "cloudy") {
    return &id(xl ? icnCloudyXL : icnCloudyMD);
  } else if (forecastVal == "rainy" 
          || forecastVal == "lightning" 
          || forecastVal == "lightning-rainy" 
          || forecastVal == "hail" 
          || forecastVal == "pouring"
          || forecastVal == "exceptional") {
    return &id(xl ? icnRainyXL : icnRainyMD);
  } else if (forecastVal == "sleet") {
    return &id(xl ? icnSleetXL : icnSleetMD);
  } else if (forecastVal == "snowy" 
          || forecastVal == "snowy-rainy") {
    return &id(xl ? icnSnowXL : icnSnowMD);
  } else if (forecastVal == "windy" 
          || forecastVal == "windy-variant") {
    return &id(xl ? icnWindXL : icnWindMD);
  } else if (forecastVal == "fog") {
    return &id(xl ? icnFogXL : icnFogMD);
  } else if (forecastVal == "partlycloudy") {
    return &id(xl ? icnPartlyCloudyXL : icnPartlyCloudyMD);
  } else {
    return &id(xl ? icnSunnyXL : icnSunnyMD);
  }
}

void forecast(int x, int y, esphome::image::Image *icnFc,
              esphome::image::Image *icnRain,
              esphome::homeassistant::HomeassistantSensor *temp,
              esphome::homeassistant::HomeassistantSensor *rain,
              display::Display &it, esphome::font::Font *font) {
  // weather condition
  it.image(x, y, icnFc);

  int fontHeight = font->get_height();
  int lineHeight = fontHeight;
  int icnFcHeight = icnFc->get_height() * .95;

  // temp
  it.printf(x + icnFc->get_width() / 2, y + icnFcHeight, font,
            TextAlign::TOP_CENTER, "%.0f °C", temp->state);
  // rain probability
  it.image((x + icnFc->get_width() / 2) - lineHeight * .75,
           y + 2 + icnFcHeight + lineHeight + fontHeight / 2, icnRain,
           ImageAlign::CENTER_RIGHT);
  it.printf(x + icnFc->get_width() / 2 + 30 / 2, y + lineHeight + icnFcHeight,
            font, TextAlign::TOP_CENTER, "%.0f %%", rain->state);
}

void plant(int x, int y, float val, esphome::image::Image *icn,
           display::Display &it, esphome::font::Font *font) {
  int progressH = 100;
  int progressW = 25;
  int margin = 5;

  it.image(x + progressW / 2, y, icn, ImageAlign::TOP_CENTER);

  // progress bar x, y, w, h
  int barX = x;
  int barY = y + icn->get_height() + margin;
  it.rectangle(barX, barY, progressW, progressH);
  it.rectangle(barX + 1, barY + 1, progressW - 2, progressH - 2);
  // remove top of bar container
  it.line(barX + 2, barY, barX + progressW - 2, barY, COLOR_OFF);
  it.line(barX + 2, barY + 1, barX + progressW - 2, barY + 1, COLOR_OFF);

  int fillHeight = 0;
  if (val > 0 && val <= 100) {
    int fillHeight = progressH * (val / 100);
    // fill bar
    it.filled_rectangle(barX, barY + progressH - fillHeight, progressW,
                        fillHeight);
  }

  // render tick-marks
  int tickInterval = 20;
  int tickLn = 10;
  for(int i = tickInterval; i < progressH; i += tickInterval) {
    if (i >= progressH - fillHeight)
      it.line(barX + 2, barY + i, barX + tickLn, barY + i, COLOR_OFF);
    else
      it.line(barX + 2, barY + i, barX + tickLn, barY + i, COLOR_ON);
  }

  // percentage
  it.printf(x + progressW / 2, y + icn->get_height() + progressH + margin * 2,
            font, TextAlign::TOP_CENTER, "%.0f%%", val);
}

void roomInfo(int x, int y, char *room,
              esphome::homeassistant::HomeassistantSensor *tempSensor,
              esphome::homeassistant::HomeassistantSensor *humidSensor,
              esphome::sensor::Sensor *internalTempSensor, 
              esphome::sensor::Sensor *internalHumidSensor,
              esphome::text_sensor::TextSensor *airSensor,
              esphome::image::Image *icnHumidity, display::Display &it,
              esphome::font::Font *headerFont, esphome::font::Font *dataFont) {
  int lineHeight = dataFont->get_height() + 1;
  int headerHeight = headerFont->get_height() + 1;

  it.printf(x, y, headerFont, "%s", room);
  if (tempSensor != NULL) {
    it.printf(x, y + headerHeight, dataFont, "%.1f °C", tempSensor->state);
  } else if (internalTempSensor != NULL) {
    it.printf(x, y + headerHeight, dataFont, "%.1f °C", internalTempSensor->state);
  }

  // humidity
  if (humidSensor != NULL) {
    it.image(x + 110, y + headerHeight + lineHeight / 2, icnHumidity, ImageAlign::CENTER_LEFT);
    it.printf(x + 110 + icnHumidity->get_width(), y + headerHeight, dataFont, "%.1f %%", humidSensor->state);
  } else if (internalHumidSensor != NULL) {
    it.image(x + 110, y + headerHeight + lineHeight / 2, icnHumidity, ImageAlign::CENTER_LEFT);
    it.printf(x + 110 + icnHumidity->get_width(), y + headerHeight, dataFont, "%.1f %%", internalHumidSensor->state);
  }

  // air
  if (airSensor != NULL) {
    it.printf(x, y + headerHeight + lineHeight, dataFont, "%s", airSensor->get_state().c_str());
  }
}

void sunSetRise(int x, int y, esphome::image::Image *icn,
                esphome::homeassistant::HomeassistantTextSensor *time,
                display::Display &it, esphome::font::Font *font) {
  it.image(x, y + font->get_height() / 2, icn, ImageAlign::CENTER_LEFT);
  if (time->has_state()) {
    it.printf(x + icn->get_width() + 5, y, font, "%s", time->state.c_str());
  } else {
    it.print(x + icn->get_width() + 5, y, font, "load...");
  }
}