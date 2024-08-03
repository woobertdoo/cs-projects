const UserAccountDetailsRepository = require("../repositories/UserAccountDetailsRepository");
const logger = require("../logger/logger");

const getAccountDetails = async (request, response) => {
    const { userId } = request.query;
    const userAccountDetailsRepository = new UserAccountDetailsRepository();
    const existingAccountDetails =
        await userAccountDetailsRepository.select(userId);
    if (existingAccountDetails) {
        logger.info(`existingUserAccountDetails ${existingAccountDetails}`);
        return response.status(200).json({
            id: existingAccountDetails.id,
            userId: existingAccountDetails.user_id,
            firstName: existingAccountDetails.first_name,
            lastName: existingAccountDetails.last_name,
            address1: existingAccountDetails.address_1,
            address2: existingAccountDetails.address_2,
            city: existingAccountDetails.city,
            state: existingAccountDetails.state,
            zip: existingAccountDetails.zip_code,
            phone: existingAccountDetails.phone_number,
            email: existingAccountDetails.email,
        });
    } else {
        const message = "No data for that account.";
        logger.info(message);
        return response.status(204).json({});
    }
};

const createAccountDetails = async (request, response) => {
    const {
        userId,
        firstName,
        lastName,
        address1,
        address2,
        city,
        state,
        zip,
        phone,
        email,
    } = request.body;

    logger.info(`USER ID: ${userId}`);

    const userAccountDetailsRepository = new UserAccountDetailsRepository();
    const userAccountDetails = await userAccountDetailsRepository.insert(
        userId,
        firstName,
        lastName,
        address1,
        address2,
        city,
        state,
        zip,
        phone,
        email,
    );
    console.log(JSON.stringify(userAccountDetails));
    return response.status(201).json({
        id: userAccountDetails.id,
    });
};

const updateAccountDetails = async (request, response) => {
    const {
        userId,
        firstName,
        lastName,
        address1,
        address2,
        city,
        state,
        zip,
        phone,
        email,
    } = request.body;

    logger.info(`userID: ${userId}`);

    const userAccountDetailsRepository = new UserAccountDetailsRepository();
    const userAccountDetails = await userAccountDetailsRepository.update(
        userId,
        firstName,
        lastName,
        address1,
        address2,
        city,
        state,
        zip,
        phone,
        email,
    );

    if (!userAccountDetails) {
        const error = "There was a problem updating your account.";
        return response.status(404).json({ error });
    } else {
        response.status(200).json({
            id: userAccountDetails.id,
            userId: userId,
            firstName: firstName,
            lastName: lastName,
            address1: address1,
            address2: address2,
            city: city,
            state: state,
            zip: zip,
            phone: phone,
            email: email,
        });
    }
};

module.exports = {
    createAccountDetails,
    getAccountDetails,
    updateAccountDetails,
};
