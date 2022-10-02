
#if defined(WIFI_HARDWARE_ATMEL) || defined(WIFI_HARDWARE_UBLOX)


//#define WIFI_DEVICE_IP      192,168,0,15   /* IP addres that is used for this device; comment to use DHCP  */

/******************************************************************************************************************************
* Connect to Wifi Access Point first.
* If that succeeds, display the IP Address on the LED panel and start waiting for incoming requests using 'WiFiServer' class.
* Another function is used to handle each incoming request.
*******************************************************************************************************************************/
void webServer() {
  if (WiFi.status() == WL_NO_SHIELD) {
    displayError("NO WIFI");  /* WIFI hardware not found :-( */
    delay(3000);
    return;
  }
  if (strlen(Settings::wifiSSID) == 0) {
    displayError("NO SSID");  /* SSID string is missing in settings :-( */
    delay(3000);
    return;    
  }
  int widthOfText = displayMsg("WAIT...", COLOR_IDX_BLUE);
//  WiFi.config(IPAddress(WIFI_DEVICE_IP));    /* comment to use DHCP instead    */
  int status = WiFi.status();

  int attempts = 0;
  while ( status != WL_CONNECTED && ++attempts <= 2) {
    int xOfAttemptNr = (PANEL_COLS + widthOfText) / 2; /* where to put the connection attempt nr? */
    ledPanel.writeChar('0' + attempts, xOfAttemptNr, COLOR_IDX_WHITE);  /* write attempt (1, 2, 3) on LED panel */
    ledPanel.writeLeds_asm();
    WiFi.begin(Settings::wifiSSID, Settings::wifiPassword);  /* Connect to WPA/WPA2 network. */
    for (int i=0; i < 10; i++) { /* check status every second for 10 seconds */
      status = WiFi.status();
      if (status == WL_CONNECTED) break; else delay(1000);
    }
  }
  if (status != WL_CONNECTED)
  {
    displayError("NO CONN.");  /* connection failed */
    delay(3000);
    return;
  }

  WiFiServer server(80);  
  server.begin();   /* start the web server on port 80 */
  
//  test_dumpWifiStatus();   /* you're connected now, so print out the status */

  bool init = true;
  while (true)
  {
    displayWifiConnectionIP(init);
    init = false;
    delay(25);  
    /* read LED panel buttons */
    ledPanel.readButtons();
    if (ledPanel.isButtonPressed(BUTTON_WIFI)) break; /* Wifi-button (button4) pressed? -> exit */
    WiFiClient client = server.available();   /* listen for incoming clients */
    if (client)
    {
      displayMsg("CLIENT...", COLOR_IDX_YELLOW);      
      handleRequest(&client);
      client.stop();
    }
  }
  WiFi.end();
  /* reload song, because a new version of this song might have been uploaded... */
  sdCard.loadSong(&song, song.songId, getSongLoadingFlags());
}

/******************************************************************************************************************************
* Read the incoming request line-by-line.
* Only 1 request type is supported: HTTP POST that includes data of a complete SONG to be saved. 
* Request data: - HTTP headers first (several lines)
*               - then "*start*" that marks start of data
*               - then one line that holds the SONG Id (1-60). This Id is used to make a unique filename for that song.
*               - then all song data, line by line
*               - then "*end*" that marks end of data
* (!!! the data is not URL-encoded which is typically done when posting data from a Web-browser. !!!)
*******************************************************************************************************************************/
#define REQUEST_MAX_LINE_LENGTH 120
void handleRequest(WiFiClient* client)
{
  File sdFile ;
  bool headerSection = true;  /* reading header lines (true) or reading data lines (false) */
  int lineNr = 0;             /* counting up, but start at 0 again where data starts */
  bool error = false;         /* when true, start writing the response */
  bool finished = false;      /* when true, start writing the response */
  char currLine[REQUEST_MAX_LINE_LENGTH + 1];
  while (client->connected()) 
  {
    if (!readRequestLine(client, currLine)) finished = true;
    if (finished || error) {               /* Finished or error?  ->  write HTTP response  */
      if (sdFile) sdFile.close();
      client->println("HTTP/1.1 200 OK");
      client->println("Content-type:text/plain");
      client->println();
      client->println(error ? "ERROR" : "OK");
      break;
    }
    lineNr++;
    /* if we are here: 'currLine' contains data (1 line) of the HTTP request */
    if (headerSection) /* we are reading header lines */
    {
      if (lineNr == 1) {  /* first HTTP header line contains the POST */
        strupr(currLine);
        if (strncmp(currLine,"POST /SAVESONG ", 15) != 0) error = true; /* not the right HTTP POST? -> error */
      }
      if (strcmp(currLine, "*start*") == 0) {  /* after this line, we get the real data, SONG Id first... */
        headerSection = false; 
        lineNr = 0;
      }
    }
    else               /* we are reading data lines */
    {
      if (strcmp(currLine, "*end*") == 0) {
        finished = true;
      }
      else if (lineNr == 1) { /* first data line: it holds the SONG Id */
        int songId = atoi(currLine);
        if (songId < 1 || songId > 60) {
          error = true;
        }
        else {
          Song::getFilename(songId, currLine);
          sdFile = SD.open(currLine, O_WRITE | O_CREAT | O_TRUNC);
          if (!sdFile) error = true;
        }
      }
      else { /* the data line is song data, that has to be written to the file */
        sdFile.println(currLine);     
      }
    }
  }
}


/******************************************************************************************************************************
* Read a line from the incoming HTTP request. Ignore empty lines.
* 
*******************************************************************************************************************************/
bool readRequestLine(WiFiClient* client, char* buf)
{
  unsigned int now = millis();
  int idx = 0;
  while (client->connected()) {
    if (client->available()) 
    {
      char c = client->read();            /* read a byte */
      if (c == '\r' || c == '\n') {
        if (idx == 0) continue;   /* ignore empty lines */
        buf[idx] = 0; /* mark end of line and exit */
        return true;
      }
      buf[idx++] = c;
      if (idx == REQUEST_MAX_LINE_LENGTH) break; /* avoid buffer overrun. */
    }
    if ((millis() - now) > 1500) break; /* reading 1 line of HTTP request cannot take more than 1.5 seconds */
  }
  buf[idx] = 0;
  return idx == 0 ? false : true; /* time-out or empty line? -> false */
}


#endif // defined(WIFI_HARDWARE_ATMEL) || defined(WIFI_HARDWARE_UBLOX)
