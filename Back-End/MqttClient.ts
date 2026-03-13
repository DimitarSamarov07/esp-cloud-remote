import mqtt, {IClientOptions, MqttClient as MqttCoreClient} from 'mqtt';
import dotenv from 'dotenv';
import {fetchAirConditionerStatus} from "./services/AirConditioner.js";
import {FanSpeed, Mode} from "./data_models/AirConditionerStatus.js";
import AirConditionerSet from "./data_models/AirConditionerSet.js";

dotenv.config();

// Enforce environment variables as strings (fallback to empty string or default to avoid 'undefined' TS errors)
const statusTopic: string = process.env.AC_STATUS_TOPIC || '';
const wifiTopicBase: string = process.env.WIFI_CONTROL_TOPIC || '';
const acControlTopicBase: string = process.env.AC_CONTROL_TOPIC || '';
const protocol: string = 'mqtt';
const url: string = process.env.MQTT_BROKER_URL || 'localhost';
const connectUrl: string = `${protocol}://${url}`;

// Type guards for your enums
function isValidMode(value: any): value is Mode {
    return Object.values(Mode).includes(value);
}

function isValidFanSpeed(value: any): value is FanSpeed {
    return Object.values(FanSpeed).includes(value);
}

// Extend the default MQTT options to ensure clientId is required
export interface MqttClientOptions extends IClientOptions {
    clientId: string;
}

class MqttClient {
    // Explicitly declare class properties for TypeScript
    private client: MqttCoreClient;
    private clientId: string;
    private subscribedTopics: Set<string>;
    private statusMap: Map<string, any>; // Consider replacing 'any' with your actual Status type
    private controllerHandlers: Record<string, Function>;

    constructor(options: MqttClientOptions) {
        this.client = mqtt.connect(connectUrl, options);
        this.clientId = options.clientId;
        this.subscribedTopics = new Set<string>();

        if (statusTopic) {
            this.subscribedTopics.add(statusTopic);
        }

        this.statusMap = new Map<string, any>();
        this.controllerHandlers = {};

        this.client.on('connect', () => {
            console.log(`[${this.clientId}] Connected`);
            if (statusTopic) {
                this.subscribe(statusTopic);
            }
        });

        // MQTT messages arrive as Buffers
        this.client.on('message', async (topic: string, message: Buffer) => {
            if (!this.subscribedTopics.has(topic)) return;
            if (topic === statusTopic) {
                await this._handleAcStatus(message);
            }
        });

        this.client.on('error', (err: Error) => {
            console.error(`[${this.clientId}] Couldn't connect because of an error:`, err.message);
            console.error(err);
        });

        this.client.on('disconnect', () => {
            console.log(`[${this.clientId}] Disconnected`);
        });
    }

    public registerControllerHandler(topic: string, handler: Function): void {
        this.controllerHandlers[topic] = handler;
    }

    /**
     * Handles JSON messages sent by the client and sends the result back to the response topic
     */
    private async _handleAcStatus(message: Buffer): Promise<void> {
        try {
            const body = JSON.parse(message.toString());
            const deviceID = body.deviceID;

            if (!deviceID) throw new Error("Missing deviceID in payload");

            const responseTopic = `ac/status/response/${deviceID}`;
            const status = await fetchAirConditionerStatus(deviceID, this);

            this.publish(responseTopic, JSON.stringify(status));
        } catch (err: any) {
            console.error(`[${this.clientId}] Failed to parse AC status or fetch data:`, err.message || err);
        }
    }

    public getStatusByDeviceId(deviceId: string): any {
        return this.statusMap.get(deviceId);
    }

    /**
     * Publishes the given message to the topic specified with QOS 1 and Retain true.
     */
    public publish(topic: string, message: string): void {
        this.client.publish(topic, message, {qos: 1, retain: true}, (error?: Error) => {
            if (error) {
                console.log(`[${this.clientId}] Publish failed:`, error.message);
            }
        });
    }

    public subscribe(topic: string): void {
        // @ts-ignore
        this.client.subscribe(topic, (error?: Error) => {
            if (error) {
                console.error(`[${this.clientId}] Subscribe error`, error);
            } else {
                this.subscribedTopics.add(topic);
            }
        });
    }

    public async changeWifi(deviceID: string, ssid: string, pass: string): Promise<void> {
        // Construct the specific topic locally rather than mutating a global variable
        const targetTopic = `${wifiTopicBase}/${deviceID}`;
        console.log(targetTopic);

        this.client.publish(targetTopic, `${ssid}/${pass}`);
        console.log(`[${this.clientId}] Sent WiFi credentials`);
    }

    public async setAcData(deviceId: string, acData: AirConditionerSet): Promise<void> {
        // Construct the specific topic locally rather than mutating a global variable
        const targetTopic = `${acControlTopicBase}/${deviceId}`;
        console.log(`[${this.clientId}] Set Ac Data`);
        console.log(JSON.stringify(acData));

        this.client.publish(targetTopic, JSON.stringify(acData));
    }
}

export default MqttClient;