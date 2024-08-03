import Header from "../components/Header";
import Footer from "../components/Footer";
import { useNavigate } from "react-router-dom";
import { useEffect } from "react";

function Logout(props) {
  const { handleLogout } = props;
  handleLogout(false);
  const navigate = useNavigate();

  useEffect(() => {
    navigate("/");
  });
}

export default Logout;
