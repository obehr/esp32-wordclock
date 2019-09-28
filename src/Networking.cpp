
#include "Networking.hpp"



const byte DNS_PORT = 53;

void Networking::loop ()
{
    if (mode == ap && apTimeout > 0 && ((millis () - apStart) / 1000) > apTimeout)
    {
        ESP.restart ();
    }

    if (dnsServer)
    {
        dnsServer->processNextRequest ();
    }
}

Networking::Mode Networking::getMode ()
{
    return this->mode;
}

void Networking::setAPName (const char *name)
{
    this->apName = (char*) name;
}

void Networking::setAPPassword (const char *password)
{
    this->apPassword = (char*) password;
}



void Networking::setAPTimeout (const int timeout)
{
    this->apTimeout = timeout;
}

void Networking::setWifiConnectRetries (const int retries)
{
    this->wifiConnectRetries = retries;
}

void Networking::setWifiConnectInterval (const int interval)
{
    this->wifiConnectInterval = interval;
}



void Networking::startAP ()
{


    mode = ap;

    Serial.println (F("Starting Access Point"));

    WiFi.mode (WIFI_AP);
    WiFi.softAP (apName, apPassword);

    delay (500); // Need to wait to get IP

    IPAddress ip (192, 168, 1, 1);
    IPAddress NMask (255, 255, 255, 0);
    WiFi.softAPConfig (ip, ip, NMask);

    IPAddress myIP = WiFi.softAPIP ();
    Serial.print ("AP IP address: ");
    Serial.println (myIP);

    dnsServer.reset (new DNSServer);
    dnsServer->setErrorReplyCode (DNSReplyCode::NoError);
    dnsServer->start (DNS_PORT, "*", ip);

    // server-on kram hier

    apStart = millis ();
}

void Networking::startApi ()
{
    const char *headerKeys[] =
        { "Content-Type" };
    size_t headerKeysSize = sizeof(headerKeys) / sizeof(char*);

    mode = api;

    // server-on kram hier
}

bool Networking::wifiConnected ()
{
    Serial.print (F("Waiting for WiFi to connect"));

    int i = 0;
    while (i < wifiConnectRetries)
    {
        if (WiFi.status () == WL_CONNECTED)
        {
            Serial.println ("");
            return true;
        }

        Serial.print (".");

        delay (wifiConnectInterval);
        i++;
    }

    Serial.println ("");
    Serial.println (F("Connection timed out"));

    return false;
}


