import mysql from 'mysql2/promise'
import dotenv from 'dotenv'

dotenv.config()
const processEnv = process.env;
export const pool = mysql.createPool({
    host: "localhost",
    user: processEnv.DB_USER,
    password: processEnv.DB_PASSWORD,
    database: processEnv.DATABASE,
    waitForConnections: true,
    connectionLimit: 10,
    maxIdle: 10, // max idle connections, the default value is the same as `connectionLimit`
    idleTimeout: 60000, // idle connections timeout, in milliseconds, the default value 60000
    queueLimit: 0,
    enableKeepAlive: true,
    keepAliveInitialDelay: 0,
});
