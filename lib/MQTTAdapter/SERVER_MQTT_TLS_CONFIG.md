# MQTT over TLS (Port 8883) Configuration for Aedes

Your ESP32 client needs MQTT over TLS (not WebSocket) to work with the PubSubClient library.

## Current Server Setup
- ✅ Port 1883: MQTT over TCP
- ✅ Path `/mqtt` on HTTPS: MQTT over WebSocket (for web clients)
- ❌ Port 8883: MQTT over TLS (needed for ESP32)

## Add MQTT over TLS to Your Aedes Server

```typescript
import { createServer as createTlsServer } from 'tls';
import { readFileSync } from 'fs';
import Aedes from 'aedes';

export function createMqttBroker(httpServer: HttpServer, mqttPort: number = 1883, mqttsPort: number = 8883, handlers?: MqttBrokerHandlers) {
  // Initialize Aedes broker
  const aedesInstance = new Aedes({
    authenticate: authenticateMqttClient,
    authorizePublish,
    authorizeSubscribe,
  });

  // 1. MQTT over TCP (port 1883) - for local/non-secure clients
  const mqttServer = createServer(aedesInstance.handle);
  mqttServer.listen(mqttPort, () => {
    console.log(`🔌 MQTT Broker (TCP) listening on port ${mqttPort}`);
  });

  // 2. MQTT over WebSocket (wss://) - for browser clients
  const wss = new WebSocketServer({
    server: httpServer,
    path: "/mqtt",
  });

  wss.on("connection", (ws, req) => {
    const stream = websocketStream(ws);
    aedesInstance.handle(stream);
    console.log(`🌐 WebSocket connection from ${req.socket.remoteAddress}`);
  });
  console.log(`🌐 MQTT WebSocket available at wss://yourdomain/mqtt`);

  // 3. MQTT over TLS (port 8883) - for ESP32 and other IoT devices
  const tlsOptions = {
    key: readFileSync('/path/to/privkey.pem'),   // Let's Encrypt private key
    cert: readFileSync('/path/to/fullchain.pem') // Let's Encrypt certificate chain
  };

  const mqttsServer = createTlsServer(tlsOptions, aedesInstance.handle);
  mqttsServer.listen(mqttsPort, () => {
    console.log(`🔒 MQTT Broker (TLS) listening on port ${mqttsPort}`);
  });

  return aedesInstance;
}
```

## Using Let's Encrypt Certificates

If you're using Let's Encrypt (e.g., via Certbot), your certificates are typically located at:
```
/etc/letsencrypt/live/yourdomain.com/privkey.pem
/etc/letsencrypt/live/yourdomain.com/fullchain.pem
```

Make sure your Node.js process has read access to these files.

## Firewall Configuration

Open port 8883 in your firewall:
```bash
# UFW (Ubuntu)
sudo ufw allow 8883/tcp

# iptables
sudo iptables -A INPUT -p tcp --dport 8883 -j ACCEPT
```

## Testing MQTT over TLS

Test from command line using mosquitto_sub:
```bash
mosquitto_sub -h server.device-manager.fast.knakitm.pl -p 8883 \
  --cafile /etc/ssl/certs/ca-certificates.crt \
  -t "test/topic" -v
```

## ESP32 Configuration

Your ESP32 is now configured to use:
- Server: `server.device-manager.fast.knakitm.pl`
- Port: `8883`
- Protocol: MQTT over TLS (MQTTS)
- Certificate validation: Disabled (`.setInsecure()`)

For production, you should enable certificate validation:
```cpp
// In MQTTAdapter.cpp constructor, replace:
wifiClientSecure.setInsecure();

// With:
wifiClientSecure.setCACert(ca_cert);
```

Where `ca_cert` is the Let's Encrypt root CA certificate.
