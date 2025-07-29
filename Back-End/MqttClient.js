import mqtt from 'mqtt';
//import AirConditioner, {FanSpeed, Mode} from "./data_models/AirConditioner.ts";
import moment from "moment";

const processEnv = process.env;
const statusTopic = processEnv.AC_STATUS_TOPIC;
const wifiTopic = processEnv.WIFI_CONTROL_TOPIC;
const protocol = 'mqtt'
const host = '93.155.224.232'
const port = '5728'

const connectUrl = `${protocol}://${host}:${port}`

function isValidMode(value) {
    return Object.values(Mode).includes(value);
}

function isValidFanSpeed(value) {
    return Object.values(FanSpeed).includes(value);
}

class MqttClient {
    constructor(options) {
        this.client = mqtt.connect(connectUrl, options)
        this.clientId = options.clientId
        this.subscribedTopics = new Set();
        this.subscribedTopics.add(statusTopic)
        this.client.on('connect', () => {
            console.log(`[${this.clientId}] Connected`);

            this.subscribe('ac/status')
        })
        this.client.on('message', (topic, message) => {
            if (!this.subscribedTopics.has(topic)) return;
            console.log(`[${this.clientId}] 📩 Message on '${topic}': ${message}`);

            if (topic === statusTopic) {
                this._handleAcStatus(message);
            }
        });

        this.client.on('error', (err) => {
            console.error(`[${this.clientId}] Couldn't connect because of an error:`, err.message);
            console.error(err); // Optional: log full stack trace
        });
        this.client.on('disconnect', () => {
            console.log(`[${this.clientId}] Disconnected`)
        })

    }

    _handleAcStatus(message) {
        try {
            const raw = JSON.parse(message.toString());

            const {
                deviceId,
                isOnline,
                isPowered,
                temperature,
                mode,
                fanSpeed,
                swing,
                lastSeen,
            } = raw;

            if (typeof deviceId !== 'string' || typeof isOnline !== 'boolean') {
                throw new Error('Invalid deviceId or isOnline field');
            }

            const parsedMode = isValidMode(mode) ? mode : null;
            const parsedFanSpeed = isValidFanSpeed(fanSpeed)
                ? fanSpeed
                : typeof fanSpeed === 'number'
                    ? fanSpeed
                    : null;
            const parsedTemp = typeof temperature === 'number' ? temperature : null;
            const parsedPower = typeof isPowered === 'boolean' ? isPowered : null;
            const parsedSwing = typeof swing === 'boolean' ? swing : null;
            const parsedLastSeen = lastSeen ? moment(lastSeen) : null;

            const status = new AirConditioner(
                deviceId,
                isOnline,
                parsedPower,
                parsedTemp,
                parsedMode,
                parsedFanSpeed,
                parsedSwing,
                parsedLastSeen
            );

            console.log(`[${this.clientId}] AC status parsed:`, status);

            if (typeof this.onAcStatusReceived === 'function') {
                this.onAcStatusReceived(status);
            }
        } catch (err) {
            console.error(`[${this.clientId}] Failed to parse AC status:`, err.message || err);
        }
    }

    publish(topic, message) {
        this.client.publish(topic, message, {qos: 1}, (error) => {
            if (error) {
                console.log(`[${this.clientId}] Publish failed`);
            } else {
                console.log(`[${this.clientId}] Published to ${topic}`);
            }
        });
    }

    subscribe(topic) {
        this.client.subscribe(topic, (error) => {
            if (error) {
                console.error(`[${this.clientId}] Subscribe error`, error);
            } else {
                console.log(`[${this.clientId}] Subscribed to ${topic}`);
                this.subscribedTopics.add(topic);
            }
        });
    }

    changeWifi(ssid, pass) {
        this.client.publish(wifiTopic, `${ssid}/${pass}`);
        console.log(`[${this.clientId}] Sent WiFi credentials`);
    }


}

export default MqttClient;

