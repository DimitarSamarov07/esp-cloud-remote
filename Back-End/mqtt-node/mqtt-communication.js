const mqtt = require('mqtt')

const protocol = 'mqtt'
const host = '93.155.224.232'
const port = '5728'
const clientId = `mqtt_${Math.random().toString(16).slice(3)}`

const connectUrl = `${protocol}://${host}:${port}`

const client = mqtt.connect(connectUrl, {
    clientId,
    clean: true,
    connectTimeout: 4000,
    username: '',
    password: '',
    reconnectPeriod: 1000,
})

const responseTopic = 'ac/report'
const controlTopic = 'ac/control'

client.on('error', (error) => {
    console.error('Connection failed\n', error)
})

client.on('reconnect', (error) => {
    console.error('Reconnection failed\n', error)
})

client.on('connect', () => {
    console.log('Connected with ID: ', clientId)

    client.subscribe([responseTopic], () => {
        console.log(`Subscribe to topic '${responseTopic}'`)
        repeatedPublishLoop(5)
    })
})

client.on('message', (topic, payload) => {
    console.log(`Received Message (${topic}): ${payload.toString()}`)
})

function repeatedPublishLoop(intervalSeconds) {
    setInterval(function () {
        let messages = ['TURN_ON', 'TURN_OFF', 'abcdef']
        let i = Math.floor(Math.random() * messages.length)

        client.publish(controlTopic, messages[i], { qos: 0, retain: false }, (error) => {
            if (error) { console.error(error) }
        })

        console.log('Published message to ', controlTopic)
    }, intervalSeconds*1000)
}