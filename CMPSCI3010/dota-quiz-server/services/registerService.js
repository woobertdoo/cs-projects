const UserAccountsRepository = require("../repositories/UserAccountsRepository");
const logger = require("../logger/logger");
const bcrypt = require("bcrypt");
const saltRounds = 10;

const registerLoginCredentials = async (request, response) => {
    const { username, password } = request.body;
    const userAccountsRepository = new UserAccountsRepository();
    const existingUserAccount = await userAccountsRepository.select(username);
    if (existingUserAccount) {
        const error = `${username} is already registered.`;
        logger.error(error);
        return response.status(409).json({ error });
    }

    logger.info("Creating new record in user_account table");
    const salt = await bcrypt.genSalt(saltRounds);
    const passwordHash = await bcrypt.hash(password, salt);
    const newUserAccount = await userAccountsRepository.insert(
        username,
        passwordHash,
    );
    const id = newUserAccount.id;
    logger.info(`Record created with id ${id}`);
    return response.status(201).json({ id });
};

module.exports = registerLoginCredentials;
