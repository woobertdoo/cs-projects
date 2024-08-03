const dbClient = require("../db/client");

class UserAccountDetailsRepository {
    constructor() {}

    async insert(
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
    ) {
        const sql = `INSERT INTO user_account_details (
                    user_id, first_name, last_name, address_1, address_2,
                    city, state, zip_code, phone_number, email)
                    VALUES ($1, $2, $3, $4, $5, $6, $7, $8, $9, $10)
                    RETURNING id`;
        const values = [
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
        ];
        const results = await dbClient.query(sql, values);
        return results.rows[0];
    }
    async update(
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
    ) {
        const sql = `UPDATE user_account_details
                     SET first_name = $1, last_name = $2,
                         address_1 = $3, address_2 = $4,
                         city = $5, state = $6,
                         zip_code = $7, phone_number = $8, email = $9
                     WHERE user_id = $10
                     RETURNING id`;
        const values = [
            firstName,
            lastName,
            address1,
            address2,
            city,
            state,
            zip,
            phone,
            email,
            userId,
        ];
        const results = await dbClient.query(sql, values);
        console.log(results.rows[0]);
        return results.rows[0];
    }

    async select(userId) {
        const sql = `SELECT id, user_id, first_name, last_name,
                          address_1, address_2, city, state,
                          zip_code, phone_number, email
                   FROM user_account_details WHERE user_id = $1`;
        const values = [userId];
        const results = await dbClient.query(sql, values);
        return results.rows[0];
    }
}

module.exports = UserAccountDetailsRepository;
