import mqtt from 'mqtt';
import dotenv from 'dotenv';
import {fetchAirConditionerStatus} from "./services/AirConditioner.js";

dotenv.config()

const processEnv = process.env;
const statusTopic = processEnv.AC_STATUS_TOPIC;// New topic for triggering Express routes
const wifiTopic = processEnv.WIFI_CONTROL_TOPIC;
const protocol = 'mqtt'
const url = processEnv.MQTT_BROKER_URL
const connectUrl = `${protocol}://${url}`

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
        this.statusMap = new Map();
        this.controllerHandlers = {};

        this.client.on('connect', () => {
            console.log(`[${this.clientId}] Connected`);
            this.subscribe(processEnv.AC_STATUS_TOPIC)
        })

        this.client.on('message', (topic, message) => {
            if (!this.subscribedTopics.has(topic)) return;
            if (topic === statusTopic) {
                this._handleAcStatus(message);
            }
        });

        this.client.on('error', (err) => {
            console.error(`[${this.clientId}] Couldn't connect because of an error:`, err.message);
            console.error(err);
        });

        this.client.on('disconnect', () => {
            console.log(`[${this.clientId}] Disconnected`)
        })
    }

    registerControllerHandler(topic, handler) {
        this.controllerHandlers[topic] = handler;
    }

    async _handleAcStatus(message) {
        try {
            const body = JSON.parse(message.toString());
            const deviceID = body.deviceID;
            const responseTopic = `ac/status/response/${deviceID}`;
            const status = await fetchAirConditionerStatus(deviceID, this); // assuming `this` is your MQTT client
            this.publish(responseTopic, JSON.stringify(status));
        } catch (err) {
            console.error(`[${this.clientId}] Failed to parse AC status or fetch data:`, err.message || err);
        }
    }

    getStatusByDeviceId(deviceId) {
        return this.statusMap.get(deviceId);
    }

    publish(topic, message) {
        this.client.publish(topic, message, {qos: 1, retain: true}, (error) => {
            if (error) {
                console.log(`[${this.clientId}] Publish failed`);
            }
        });
    }

    subscribe(topic) {
        this.client.subscribe(topic, (error) => {
            if (error) {
                console.error(`[${this.clientId}] Subscribe error`, error);
            } else {
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