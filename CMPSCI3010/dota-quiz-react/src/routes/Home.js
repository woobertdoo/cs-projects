import { Link } from "react-router-dom";
import Header from "../components/Header";
import Footer from "../components/Footer";

function Home(props) {
  const { loggedIn } = props;
  return (
    <div>
      <Header />
      <div className="row d-flex justify-content-center">
        <ul id="nav" className="col-10 col-lg-6">
          {!loggedIn && (
            <>
              <li className="nav-button">
                <Link
                  className="d-flex align-items-center justify-content-center"
                  to="/login"
                >
                  Login
                </Link>
              </li>
              <li className="nav-button">
                <Link
                  className="d-flex align-items-center justify-content-center"
                  to="/register"
                >
                  Register
                </Link>
              </li>
            </>
          )}
          {loggedIn && (
            <>
              <li className="nav-button">
                <Link
                  className="d-flex align-items-center justify-content-center"
                  to="/account"
                >
                  Account
                </Link>
              </li>
              <li className="nav-button">
                <Link
                  className="d-flex align-items-center justify-content-center"
                  to="/logout"
                >
                  Logout
                </Link>
              </li>
            </>
          )}
        </ul>
      </div>
      <Footer />
    </div>
  );
}

export default Home;
