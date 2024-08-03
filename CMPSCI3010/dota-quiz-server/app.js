const express = require("express");
const app = express();
const cors = require("cors");
const logger = require("./logger/logger");
const validateLoginCredentials = require("./services/loginService");
const registerLoginCredentials = require("./services/registerService");
const {
    getAccountDetails,
    createAccountDetails,
    updateAccountDetails,
} = require("./services/accountService.js");
const port = 8080;

app.use(express.json({ limit: "100mb" }));
app.use(express.urlencoded({ extended: true }));
app.use(cors());
app.use((request, response, next) => {
    console.log("A new request received at " + new Date(Date.now()));
    next();
});

app.post("/login", (request, response) =>
    validateLoginCredentials(request, response),
);

app.post("/register", (request, response) =>
    registerLoginCredentials(request, response),
);

app.post("/account", (request, response) => {
    createAccountDetails(request, response);
});

app.get("/account", (request, response) => {
    getAccountDetails(request, response);
});

app.put("/account", (request, response) =>
    updateAccountDetails(request, response),
);

app.listen(port, () => {
    console.log(`Tutorial app listening on port ${port}`);
});
