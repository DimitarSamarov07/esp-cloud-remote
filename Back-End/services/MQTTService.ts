import MqttClient from './../MqttClient.ts';
import dotenv from "dotenv";

dotenv.config();
const processEnv = process.env;

const esp = new MqttClient({
    clientId: 'server',
    username: processEnv.MQTT_USERNAME,
    password: processEnv.MQTT_PASSWORD,
    clean: true, // Use true for testing to avoid strict session requirement rejections
    reconnectPeriod: 5000,
    protocolVersion: 5, // Fallback to v3.1.1 to be safe
});

export default esp;