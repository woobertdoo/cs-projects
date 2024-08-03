const Client = require("pg");

const connectionOptions = {
    host: "localhost",
    port: 5432,
    user: "postgres",
    password: "postgres",
    database: "postgres",
};
const dbClient = new Client.Client(connectionOptions);

dbClient.connect(() => {
    console.log("Connected to postgres db!");
});

module.exports = dbClient;
