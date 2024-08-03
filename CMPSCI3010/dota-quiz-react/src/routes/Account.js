import Header from "../components/Header";
import Footer from "../components/Footer";
import { useEffect, useState, useRef } from "react";

function useDidMount() {
  const mountRef = useRef(true);
  useEffect(() => {
    mountRef.current = false;
  }, []);
  return mountRef.current;
}

function Account(props) {
  const [firstName, setFirstName] = useState("");
  const [lastName, setLastName] = useState("");
  const [address1, setAddress1] = useState("");
  const [address2, setAddress2] = useState("");
  const [city, setCity] = useState("");
  const [state, setState] = useState("chooose");
  const [zip, setZip] = useState("");
  const [phone, setPhone] = useState("");
  const [email, setEmail] = useState("");
  const [firstUse, setFirstUse] = useState(true);
  const { userId, setID } = props;

  const loadData = async () => {
    console.log(userId);
    const response = await fetch(
      `http://localhost:8080/account?userId=${userId}`,
      {
        method: "GET",
      },
    );

    const status = response.status;

    if (status === 200) {
      const responseJSON = await response.json();
      setFirstName(responseJSON.firstName);
      setLastName(responseJSON.lastName);
      setAddress1(responseJSON.address1);
      setAddress2(responseJSON.address2);
      setCity(responseJSON.city);
      setState(responseJSON.state);
      setZip(responseJSON.zip);
      setPhone(responseJSON.phone);
      setEmail(responseJSON.email);
      console.log(responseJSON.userId);
      setID(responseJSON.userId);
      setFirstUse(false);
    } else if (status === 204) {
      console.log("No account data to fill");
    } else {
      alert("Error loading account data");
    }
  };

  const handleChange = (event) => {
    const name = event.target.name;
    const value = event.target.value;

    switch (name) {
      case "fname":
        setFirstName(value);
        break;
      case "lname":
        setLastName(value);
        break;
      case "add-line1":
        setAddress1(value);
        break;
      case "add-line2":
        setAddress2(value);
        break;
      case "city":
        setCity(value);
        break;
      case "state":
        setState(value);
        break;
      case "zip":
        setZip(value);
        break;
      case "phone":
        setPhone(value);
        break;
      case "email":
        setEmail(value);
        break;
      default:
        alert("Something went wrong updating");
        break;
    }
  };

  const handleSubmit = async (event) => {
    let method = "";
    if (firstUse) {
      method = "POST";
    } else {
      method = "PUT";
    }

    try {
      event.preventDefault();
      const response = await fetch("http://localhost:8080/account", {
        method: method,
        headers: {
          "Content-Type": "application/json",
        },
        body: JSON.stringify({
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
        }),
      });
      const status = response.status;
      if (status === 200 || status === 201) {
        const responseJSON = await response.json();
        console.log(`responseJSON status ${status}: ${responseJSON}`);
        setID(responseJSON.userId);
        setFirstUse(false);
      } else if (status === 404) {
        alert("Error saving account data");
      }
    } catch (error) {
      alert(`Something went wrong: ${error.message}`);
    }
  };

  if (useDidMount()) {
    console.log("mounted");
    loadData();
  }

  return (
    <>
      <Header />
      <form
        className="account-info nanum-gothic-coding-regular"
        onSubmit={handleSubmit}
      >
        <div className="row">
          <label
            className="col-3 offset-1 col-md-3 offset-md-2 col-lg-2 offset-lg-3"
            htmlFor="fname"
          >
            First Name:
          </label>
          <input
            className="col-3 col-md-1"
            type="text"
            id="fname"
            name="fname"
            value={firstName}
            onChange={handleChange}
          />
        </div>
        <div className="row">
          <label
            className="col-3 offset-1 col-md-3 offset-md-2 col-lg-2 offset-lg-3"
            htmlFor="lname"
          >
            Last Name:
          </label>
          <input
            className="col-3 col-md-1"
            type="text"
            id="lname"
            name="lname"
            value={lastName}
            onChange={handleChange}
          />
        </div>
        <div className="row">
          <label
            className="col-3 offset-1 col-md-3 offset-md-2 col-lg-2 offset-lg-3"
            htmlFor="add-line1"
          >
            Address 1:
          </label>
          <input
            className="col-3 col-md-1"
            type="text"
            id="add-line1"
            name="add-line1"
            value={address1}
            onChange={handleChange}
          />
        </div>
        <div className="row">
          <label
            className="col-3 offset-1 col-md-3 offset-md-2 col-lg-2 offset-lg-3"
            htmlFor="add-line2"
          >
            Address 2:
          </label>
          <input
            className="col-3 col-md-1"
            type="text"
            id="add-line2"
            name="add-line2"
            value={address2}
            onChange={handleChange}
          />
        </div>
        <div className="row">
          <label
            className="col-3 offset-1 col-md-3 offset-md-2 col-lg-2 offset-lg-3"
            htmlFor="city"
          >
            City:
          </label>
          <input
            className="col-3 col-md-1"
            type="text"
            id="city"
            name="city"
            value={city}
            onChange={handleChange}
          />
        </div>
        <div className="row">
          <label
            className="col-3 offset-1 col-md-3 offset-md-2 col-lg-2 offset-lg-3"
            htmlFor="state"
          >
            State:
          </label>
          <select
            className="col-5 col-sm-3 col-lg-2"
            id="state"
            name="state"
            value={state}
            onChange={handleChange}
          >
            <option value="choose">Choose a state...</option>
            <option value="alabama">Alabama</option>
            <option value="alaska">Alaska</option>
            <option value="arizona">Arizona</option>
            <option value="arkansas">Arkansas</option>
            <option value="california">California</option>
            <option value="colorado">Colorado</option>
            <option value="connecticut">Connecticut</option>
            <option value="delaware">Delaware</option>
            <option value="florida">Florida</option>
            <option value="georgia">Georgia</option>
            <option value="hawaii">Hawaii</option>
            <option value="idaho">Idaho</option>
            <option value="illinois">Illinois</option>
            <option value="indiana">Indiana</option>
            <option value="iowa">Iowa</option>
            <option value="kansas">Kansas</option>
            <option value="kentucky">Kentucky</option>
            <option value="louisiana">Louisiana</option>
            <option value="maine">Maine</option>
            <option value="maryland">Maryland</option>
            <option value="massachusetts">Massachusetts</option>
            <option value="michigan">Michigan</option>
            <option value="minnesota">Minnesota</option>
            <option value="mississippi">Mississippi</option>
            <option value="missouri">Missouri</option>
            <option value="montana">Montana</option>
            <option value="nebraska">Nebraska</option>
            <option value="nevada">Nevada</option>
            <option value="new-hampshire">New Hampshire</option>
            <option value="new-mexico">New Mexico</option>
            <option value="new-york">New York</option>
            <option value="north-carolina">North Carolina</option>
            <option value="north-dakota">North Dakota</option>
            <option value="ohio">Ohio</option>
            <option value="oklahoma">Oklahoma</option>
            <option value="oregon">Oregon</option>
            <option value="pennsylvania">Pennsylvania</option>
            <option value="rhode-island">Rhode Island</option>
            <option value="south-carolina">South Carolina</option>
            <option value="south-dakota">South Dakota</option>
            <option value="tennessee">Tennessee</option>
            <option value="texas">Texas</option>
            <option value="utah">Utah</option>
            <option value="vermont">Vermont</option>
            <option value="virginia">Virginia</option>
            <option value="washington">Washington</option>
            <option value="west-virginia">West Virginia</option>
            <option value="wisconsin">Wisconsin</option>
            <option value="wyoming">Wyoming</option>
          </select>
        </div>
        <div className="row">
          <label
            className="col-3 offset-1 col-md-3 offset-md-2 col-lg-2 offset-lg-3"
            htmlFor="zip"
          >
            Zip Code:
          </label>
          <input
            className="col-3 col-md-1"
            type="text"
            id="zip"
            name="zip"
            value={zip}
            onChange={handleChange}
          />
        </div>
        <div className="row">
          <label
            className="col-3 offset-1 col-md-3 offset-md-2 col-lg-2 offset-lg-3"
            htmlFor="phone"
          >
            Phone Number:
          </label>
          <input
            className="col-3 col-md-1"
            type="text"
            id="phone"
            name="phone"
            value={phone}
            onChange={handleChange}
          />
        </div>
        <div className="row">
          <label
            className="col-3 offset-1 col-md-3 offset-md-2 col-lg-2 offset-lg-3"
            htmlFor="email"
          >
            Email:
          </label>
          <input
            className="col-3 col-md-1"
            type="text"
            id="email"
            name="email"
            value={email}
            onChange={handleChange}
          />
        </div>
        <div className="d-flex justify-content-center align-items-center">
          <input type="submit" id="submit" name="submit" value="Save" />
          <input type="reset" name="reset" id="reset" value="Reset" />
        </div>
      </form>
      <Footer />
    </>
  );
}

export default Account;
