import Header from "../components/Header";
import Footer from "../components/Footer";
import { useNavigate } from "react-router-dom";
import { useState } from "react";

function Login(props) {
  const [username, setUsername] = useState("");
  const [password, setPassword] = useState("");
  const { handleLogin } = props;
  const { setID } = props;
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
      const response = await fetch("http://localhost:8080/login", {
        method: "POST",
        headers: {
          "Content-Type": "application/json",
        },
        body: JSON.stringify({ username, password }),
      });
      const status = response.status;
      const responseJSON = await response.json();
      console.log("responseJSON", responseJSON);
      if (status === 200) {
        navigate("/");
        console.log(`id: ${responseJSON.id}`);
        setID(responseJSON.id);
        handleLogin(true);
      } else {
        alert("Wrong username/password!");
      }
    } catch (error) {
      alert(`Something went wrong: ${error.message}`);
    }
  };

  return (
    <div>
      <Header />
      <form
        className="login nanum-gothic-coding-regular"
        onSubmit={handleSubmit}
      >
        <div className="row">
          <label
            className="col-3 offset-1 col-md-2 offset-md-3 col-lg-1 offset-lg-4 d-flex justify-content-end"
            htmlFor="username"
          >
            Username:
          </label>
          <input
            className="col-3 col-md-1"
            type="text"
            value={username || ""}
            id="username"
            name="username"
            onChange={handleChange}
          />
        </div>
        <div className="row">
          <label
            className="col-3 offset-1 col-md-2 offset-md-3 col-lg-1 offset-lg-4 d-flex justify-content-end"
            htmlFor="password"
          >
            Password:
          </label>
          <input
            className="col-3 col-md-1"
            type="password"
            value={password || ""}
            id="password"
            name="password"
            onChange={handleChange}
          />
        </div>
        <div className="d-flex justify-content-center align-items-center">
          <input type="submit" id="submit" name="submit" value="Log In" />
          <input type="reset" name="reset" id="reset" value="Reset" />
        </div>
      </form>
      <Footer />
    </div>
  );
}

export default Login;
