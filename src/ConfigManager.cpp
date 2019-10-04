#include "ConfigManager.h"

const uint8_t ConfigManager::WIFI_OFFSET   = 2;
const uint8_t ConfigManager::CONFIG_OFFSET = 98;

const char ConfigManager::magicBytes[]     PROGMEM = { 'C', 'M' };
const char ConfigManager::mimeHTML[]       PROGMEM = "text/html";
const char ConfigManager::mimeJSON[]       PROGMEM = "application/json";
const char ConfigManager::mimePlain[]      PROGMEM = "text/plain";
const char ConfigManager::configHTMLFile[] PROGMEM = "/settings.html";
const char ConfigManager::apFilename[]     PROGMEM = "/index.html";

void ConfigManager::createCustomRoute (WebServer& server)
{
    server.on ("/settings.html", HTTPMethod::HTTP_GET, [&server] ()
        {
            SPIFFS.begin();

            File f = SPIFFS.open(ConfigManager::configHTMLFile, "r");
            if (!f)
            {
                Serial.println(F("file open failed"));
                server.send(404, FPSTR(ConfigManager::mimeHTML), F("File not found 5"));
                return;
            }

            server.streamFile(f, FPSTR(ConfigManager::mimeHTML));

            f.close();
        }
    );
}

ConfigManager::ConfigManager(WebServer& server, Networking& net) : server(server), net(net)
{
    config = 0;
    configSize = 0;

    setAPICallback (createCustomRoute);
    setAPCallback (createCustomRoute);

    const char *headerKeys[] = { "Content-Type" };
    size_t headerKeysSize = sizeof(headerKeys) / sizeof(char*);

    //server.reset (new WebServer (80)); // Beim Zugriff auf server hauts ihn raus -> da passt was mit der Übergabe nicht
    server.collectHeaders (headerKeys, headerKeysSize);
    server.on ("/", HTTPMethod::HTTP_GET, std::bind (&ConfigManager::handleAPGet, this));
    server.on ("/", HTTPMethod::HTTP_POST, std::bind (&ConfigManager::handleAPPost, this));
    server.on ("/jquery-3.4.1.min.js", HTTPMethod::HTTP_GET, std::bind (&ConfigManager::handleJQueryGet, this));
    server.on ("/jquery-1.1.9.1.validate.min.js", HTTPMethod::HTTP_GET,
                std::bind (&ConfigManager::handleJQueryValidateGet, this));
    server.on ("/settings", HTTPMethod::HTTP_GET, std::bind (&ConfigManager::handleRESTGet, this));
    server.on ("/settings", HTTPMethod::HTTP_PUT, std::bind (&ConfigManager::handleRESTPut, this));
    server.onNotFound (std::bind (&ConfigManager::handleNotFound, this));

    if (apCallback)
    {
        apCallback (server);
    }
}

void ConfigManager::loop ()
{

}

void ConfigManager::save ()
{
    this->writeConfig ();
}

void ConfigManager::setAPCallback (std::function<void (WebServer&)> callback)
{
    this->apCallback = callback;
}

void ConfigManager::setAPICallback (std::function<void (WebServer&)> callback)
{
    this->apiCallback = callback;
}



JsonObject&
ConfigManager::decodeJson (String jsonString)
{
    DynamicJsonBuffer jsonBuffer;

    if (jsonString.length () == 0)
    {
        return jsonBuffer.createObject ();
    }

    JsonObject &obj = jsonBuffer.parseObject (jsonString);

    if (!obj.success ())
    {
        return jsonBuffer.createObject ();
    }

    return obj;
}

void ConfigManager::handleAPGet ()
{
    SPIFFS.begin ();

    File f = SPIFFS.open (apFilename, "r");
    if (!f)
    {
        Serial.println (F("file open failed"));
        server.send (404, FPSTR(mimeHTML), F("File not found"));
        return;
    }

    server.streamFile (f, FPSTR(mimeHTML));

    f.close ();
}

void ConfigManager::handleJQueryValidateGet ()
{
    SPIFFS.begin ();

    File f = SPIFFS.open ("/jquery-1.1.9.1.validate.min.js", "r");
    if (!f)
    {
        //DebugPrintln(F("file open failed"));
        server.send (404, FPSTR(mimeHTML), F("File not found"));
        return;
    }

    server.streamFile (f, FPSTR(mimeHTML));

    f.close ();
}

void ConfigManager::handleJQueryGet ()
{
    SPIFFS.begin ();

    File f = SPIFFS.open ("/jquery-3.4.1.min.js", "r");
    if (!f)
    {
        //DebugPrintln(F("file open failed"));
        server.send (404, FPSTR(mimeHTML), F("File not found"));
        return;
    }

    server.streamFile (f, FPSTR(mimeHTML));

    f.close ();
}

void ConfigManager::handleAPPost ()
{
    bool isJson = server.header ("Content-Type") == FPSTR(mimeJSON);
    String ssid;
    String password;
    char ssidChar[32];
    char passwordChar[64];

    if (isJson)
    {
        JsonObject &obj = this->decodeJson (server.arg ("plain"));

        ssid = obj.get<String> ("ssid");
        password = obj.get<String> ("password");
    }
    else
    {
        ssid = server.arg ("ssid");
        password = server.arg ("password");
    }

    if (ssid.length () == 0)
    {
        server.send (400, FPSTR(mimePlain), F("Invalid ssid or password."));
        return;
    }

    strncpy (ssidChar, ssid.c_str (), sizeof(ssidChar));
    strncpy (passwordChar, password.c_str (), sizeof(passwordChar));

    EEPROM.put (0, magicBytes);
    EEPROM.put (WIFI_OFFSET, ssidChar);
    EEPROM.put (WIFI_OFFSET + 32, passwordChar);
    EEPROM.commit ();

    server.send (204, FPSTR(mimePlain), F("Saved. Will attempt to reboot."));

    ESP.restart ();
}

void ConfigManager::handleRESTGet ()
{
    DynamicJsonBuffer jsonBuffer;
    JsonObject &obj = jsonBuffer.createObject ();

    std::list<BaseParameter*>::iterator it;
    for (it = parameters.begin (); it != parameters.end (); ++it)
    {
        if ((*it)->getMode () == BaseParameter::set)
        {
            continue;
        }

        (*it)->toJson (&obj);
    }

    String body;
    obj.printTo (body);

    server.send (200, FPSTR(mimeJSON), body);
}

void ConfigManager::handleRESTPut ()
{
    JsonObject &obj = this->decodeJson (server.arg ("plain"));
    if (!obj.success ())
    {
        server.send (400, FPSTR(mimeJSON), "");
        return;
    }

    std::list<BaseParameter*>::iterator it;
    for (it = parameters.begin (); it != parameters.end (); ++it)
    {
        if ((*it)->getMode () == BaseParameter::ParameterMode::get)
        {
            continue;
        }

        (*it)->fromJson (&obj);
    }

    writeConfig ();

    server.send (204, FPSTR(mimeJSON), "");
}

void ConfigManager::handleNotFound ()
{
    if (!isIp (server.hostHeader ()))
    {
        server.sendHeader ("Location", String ("http://") + toStringIP (server.client ().localIP ()), true);
        server.send (302, FPSTR(mimePlain), ""); // Empty content inhibits Content-length header so we have to close the socket ourselves.
        server.client ().stop ();
        return;
    }

    server.send (404, FPSTR(mimePlain), "");
    server.client ().stop ();
}

boolean ConfigManager::isIp (String str)
{
    for (int i = 0; i < str.length (); i++)
    {
        int c = str.charAt (i);
        if (c != '.' && (c < '0' || c > '9'))
        {
            return false;
        }
    }
    return true;
}

String ConfigManager::toStringIP (IPAddress ip)
{
    String res = "";
    for (int i = 0; i < 3; i++)
    {
        res += String ((ip >> (8 * i)) & 0xFF) + ".";
    }
    res += String (((ip >> 8 * 3)) & 0xFF);
    return res;
}



void ConfigManager::setup ()
{
    char magic[2];
    char ssid[32];
    char password[64];

    Serial.println (F("Reading saved configuration"));

    EEPROM.get (0, magic);
    EEPROM.get (WIFI_OFFSET, ssid);
    EEPROM.get (WIFI_OFFSET + 32, password);
    readConfig ();

    if (memcmp (magic, magicBytes, 2) == 0)
    {
        WiFi.begin (ssid, password[0] == '\0' ? NULL : password);
        if (net.wifiConnect())
        {
            Serial.print (F("Connected to "));
            Serial.print (ssid);
            Serial.print (F(" with "));
            Serial.println (WiFi.localIP ());

            WiFi.mode (WIFI_STA);
            WiFi.setSleep (true);
            net.startApi ();
            return;
        }
    }
    else
    {
        // We are at a cold start, don't bother timeing out.
        net.apTimeout = 0;
    }

    net.startAP ();
}


void ConfigManager::readConfig ()
{
    byte *ptr = (byte*) config;

    for (int i = 0; i < configSize; i++)
    {
        *(ptr++) = EEPROM.read (CONFIG_OFFSET + i);
    }
}

void ConfigManager::writeConfig ()
{
    byte *ptr = (byte*) config;

    for (int i = 0; i < configSize; i++)
    {
        EEPROM.write (CONFIG_OFFSET + i, *(ptr++));
    }
    EEPROM.commit ();
}


