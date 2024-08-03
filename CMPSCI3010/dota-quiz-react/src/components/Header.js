import dotaLogo from "../Dota-2-Logo.png";
import { Link } from "react-router-dom";

function Header() {
  return (
    <Link to="/" className="row d-flex web-heading nanum-myeongjo-regular">
      <h1 className="col-6 col-md-5  d-flex justify-content-end align-items-center">
        DOTA
      </h1>
      <div className="col d-none d-md-flex align-items-center" id="header-icon">
        <img src={dotaLogo} alt="Dota 2 Logo" />
      </div>
      <h1 className="col-6 col-md-5 d-flex justify-content-start align-items-center">
        QUIZ
      </h1>
    </Link>
  );
}

export default Header;
