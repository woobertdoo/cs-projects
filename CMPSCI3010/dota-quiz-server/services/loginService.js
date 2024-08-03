const UserAccountsRepository = require("../repositories/UserAccountsRepository");
const logger = require("../logger/logger");
const bcrypt = require("bcrypt");

const validateLoginCredentials = async (request, response) => {
    const { username, password } = request.body;
    const userAccountsRepository = new UserAccountsRepository();
    const existingUserAccount = await userAccountsRepository.select(username);

    if (!existingUserAccount) {
        const error = `${username} does not have an account.`;
        logger.error(error);
        return response.status(400).json({ error });
    }

    logger.info(`existingUserAccount ${existingUserAccount.user_name}`);
    const isMatch = await bcrypt.compare(
        password,
        existingUserAccount.password,
    );

    if (!isMatch) {
        const error = `Password is incorrect.`;
        logger.error(error);
        return response.status(401).json({ error });
    }
    const id = existingUserAccount.id;
    logger.info(`Authentication successful!`);
    return response.status(200).json({ id });
};

module.exports = validateLoginCredentials;
