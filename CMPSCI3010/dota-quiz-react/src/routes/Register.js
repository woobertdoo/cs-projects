import Header from "../components/Header";
import Footer from "../components/Footer";
import { useState } from "react";
import { useNavigate } from "react-router-dom";

function Register(props) {
  const [username, setUsername] = useState("");
  const [password, setPassword] = useState("");
  const { handleRegister, setID } = props;
  const navigate = useNavigate();

  const handleChange = (event) => {
    const name = event.target.name;
    const value = event.target.value;

    if (name === "username") setUsername(value);
    else if (name === "password") setPassword(value);
  };

  const handleSubmit = async (event) => {
    try {
      event.preventDefault();
      handleRegister(true);
      const response = await fetch("http://localhost:8080/register", {
        method: "POST",
        headers: {
          "Content-Type": "application/json",
        },
        body: JSON.stringify({ username, password }),
      });
      const status = response.status;
      const responseJSON = await response.json();
      console.log("responseJSON", responseJSON);
      if (status === 201) {
        handleRegister(true);
        setID(responseJSON.id);
        navigate("/");
      } else {
        alert(`${responseJSON.user_name} is already registered.`);
      }
    } catch (error) {
      alert(`Something went wrong: ${error.message}`);
    }
  };

  return (
    <>
      <Header />
      <form
        className="register nanum-gothic-coding-regular"
        onSubmit={handleSubmit}
      >
        <div className="row">
          <label
            className="col-3 offset-1 col-md-3 offset-md-2 col-lg-1 offset-lg-4 d-flex justify-content-end"
            for="username"
          >
            Username:
          </label>
          <input
            type="text"
            id="username"
            name="username"
            onChange={handleChange}
          />
        </div>
        <div className="row">
          <label
            className="col-3 offset-1 col-md-3 offset-md-2 col-lg-1 offset-lg-4 d-flex justify-content-end"
            for="password"
          >
            Password:
          </label>
          <input
            type="password"
            id="password"
            name="password"
            onChange={handleChange}
          />
        </div>
        <div className="row">
          <label
            className="col-3 offset-1 col-md-4 offset-md-1 col-lg-1 offset-lg-4 d-flex justify-content-end"
            for="confirm-password"
          >
            Confirm Password:
          </label>
          <input
            type="password"
            id="confirm-password"
            name="confirm-password"
          />
        </div>
        <div className="col d-flex justify-content-center align-items-center">
          <input type="submit" id="submit" name="submit" value="Register" />
          <input type="reset" name="reset" id="reset" value="Reset" />
        </div>
      </form>
      <Footer />
    </>
  );
}

export default Register;
