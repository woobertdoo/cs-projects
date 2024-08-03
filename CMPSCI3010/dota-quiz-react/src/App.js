import "./App.css";
import Home from "./routes/Home";
import { BrowserRouter, Routes, Route } from "react-router-dom";
import Layout from "./routes/Layout";
import Login from "./routes/Login";
import { useState } from "react";
import Logout from "./routes/Logout";
import Register from "./routes/Register";
import Account from "./routes/Account";

function App() {
  const [loggedIn, setLoggedIn] = useState(false);
  const [userID, setUserID] = useState("");

  const handleUserSession = (isLoggedIn) => {
    setLoggedIn(isLoggedIn);
  };
  return (
    <BrowserRouter>
      <Routes>
        <Route path="/" element={<Layout />}>
          <Route index element={<Home loggedIn={loggedIn} />} />
          <Route
            path="login"
            element={
              <Login handleLogin={handleUserSession} setID={setUserID} />
            }
          />
          <Route
            path="logout"
            element={<Logout handleLogout={handleUserSession} />}
          />
          <Route
            path="register"
            element={
              <Register handleRegister={handleUserSession} setID={setUserID} />
            }
          />
          <Route
            path="account"
            element={<Account userId={userID} setID={setUserID} />}
          />
        </Route>
      </Routes>
    </BrowserRouter>
  );
}

export default App;
