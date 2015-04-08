package controllers

import play.api._
import play.api.mvc._
import play.api.data._
import play.api.data.Forms._

object Application extends Controller {

    def index = Action {
        Ok("Welcome page")
    }
  
    def connexion(pseudo : String, password : String) = Action {
         if (pseudo == "kudly" && password == "kudly"){
             Ok("The connexion succeeded")
         } else {
             Ok("Wrong pseudo/password")
         }
    }

    def pwm = Action {
        Ok("<led>\n<pwm_set n=\"1\" r=\"255\" g=\"125\" b=\"3\"/>\n</led>")
    }
}
