import mqtt, {
    IClientOptions,
    MqttClient as MqttCoreClient,
    IClientPublishOptions,
    ISubscriptionGrant
} from 'mqtt';
import dotenv from 'dotenv';
import { fetchAirConditionerStatus } from "./services/data/AirConditioner.ts";
import {AirConditionerSet, FanSpeed,Mode} from "./data_models/AirConditioner.ts";

dotenv.config();

// Environment variables
const statusTopic: string = process.env.AC_STATUS_TOPIC || '';
const wifiTopicBase: string = process.env.WIFI_CONTROL_TOPIC || '';
const acControlTopicBase: string = process.env.AC_CONTROL_TOPIC || '';
const protocol: string = 'mqtt';
const url: string = process.env.MQTT_BROKER_URL || 'localhost';
const connectUrl: string = `${protocol}://${url}`;

// Exported type guards
export function isValidMode(value: unknown): value is Mode {
    return Object.values(Mode).includes(value as Mode);
}

export function isValidFanSpeed(value: unknown): value is FanSpeed {
    return Object.values(FanSpeed).includes(value as FanSpeed);
}

export interface MqttClientOptions extends IClientOptions {
    clientId: string;
}

export type ControllerHandler = (topic: string, message: Buffer) => void | Promise<void>;

class MqttClient {
    // The ! tells TypeScript this will be initialized later in the connect() method
    private client!: MqttCoreClient;
    private options: MqttClientOptions;
    private clientId: string;
    private subscribedTopics: Set<string>;
    private statusMap: Map<string, unknown>;
    private controllerHandlers: Record<string, ControllerHandler>;

    constructor(options: MqttClientOptions) {
        // Store the options and initialize maps, but DO NOT connect yet.
        this.options = options;
        this.clientId = options.clientId;
        this.subscribedTopics = new Set<string>();
        this.statusMap = new Map<string, unknown>();
        this.controllerHandlers = {};
    }

    /**
     * Establishes the connection to the MQTT Broker.
     * Must be called AFTER the Express server is listening.
     */
    public connect(): void {
        this.client = mqtt.connect(connectUrl, this.options);

        this.client.on('connect', () => {
            console.log(`[${this.clientId}] Connected to broker`);
            if (statusTopic) {
                this.subscribe(statusTopic);
            }
        });

        this.client.on('message', async (topic: string, message: Buffer) => {
            if (!this.subscribedTopics.has(topic)) return;

            if (topic === statusTopic) {
                await this._handleAcStatus(message);
            }

            // Execute any registered custom handlers for this topic
            const handler = this.controllerHandlers[topic];
            if (handler) {
                await handler(topic, message);
            }
        });

        this.client.on('error', (err: Error) => {
            console.error(`[${this.clientId}] Connection error:`, err.message);
        });

        this.client.on('disconnect', () => {
            console.log(`[${this.clientId}] Disconnected from broker`);
        });
    }

    public registerControllerHandler(topic: string, handler: ControllerHandler): void {
        this.controllerHandlers[topic] = handler;
        // Since we might register handlers before connecting, we shouldn't force subscribe here yet.
        // If needed, you can add logic to subscribe dynamically once connected.
    }

    private async _handleAcStatus(message: Buffer): Promise<void> {
        try {
            const body = JSON.parse(message.toString());
            const deviceID = body.deviceID;

            if (!deviceID) {
                throw new Error("Missing deviceID in payload");
            }

            const responseTopic = `ac/status/response/${deviceID}`;
            const status = await fetchAirConditionerStatus(deviceID, this);

            // Save the latest status to our map
            this.statusMap.set(deviceID, status);

            this.publish(responseTopic, JSON.stringify(status));
        } catch (err) {
            const errorMessage = err instanceof Error ? err.message : String(err);
            console.error(`[${this.clientId}] Failed to parse AC status:`, errorMessage);
        }
    }

    public getStatusByDeviceId(deviceId: string): unknown {
        return this.statusMap.get(deviceId);
    }

    public publish(
        topic: string,
        message: string | Buffer,
        options: IClientPublishOptions = { qos: 1, retain: true }
    ): void {
        if (!this.client) {
            console.error(`[${this.clientId}] Cannot publish, client is not connected.`);
            return;
        }

        this.client.publish(topic, message, options, (error?: Error) => {
            if (error) {
                console.error(`[${this.clientId}] Publish failed on topic ${topic}:`, error.message);
            }
        });
    }

    public subscribe(topic: string): void {
        if (!this.client) return;

        this.client.subscribe(topic, (error: Error | null, granted: ISubscriptionGrant[]) => {
            if (error) {
                console.error(`[${this.clientId}] Subscribe error on ${topic}:`, error.message);
            } else {
                this.subscribedTopics.add(topic);
                console.log(`[${this.clientId}] Subscribed to ${topic}`);
            }
        });
    }

    public async changeWifi(deviceID: string, ssid: string, pass: string): Promise<void> {
        const targetTopic = `${wifiTopicBase}/${deviceID}`;
        console.log(`[${this.clientId}] Sending WiFi credentials to ${targetTopic}`);
        this.publish(targetTopic, `${ssid}/${pass}`);
    }

    public async setAcData(deviceId: string, acData: AirConditionerSet): Promise<void> {
        const targetTopic = `${acControlTopicBase}/${deviceId}`;
        console.log(`[${this.clientId}] Setting AC Data for ${deviceId}`);
        this.publish(targetTopic, JSON.stringify(acData));
    }
}

export default MqttClient;