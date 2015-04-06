package controllers

import play.api._
import play.api.mvc._
import play.api.data._
import play.api.data.Forms._
import java.io.File
import java.io.FileInputStream
import play.api.libs.iteratee._
import scala.concurrent.ExecutionContext.Implicits.global


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

    /*
     * Send a simple test for leds
     */
    def pwm = Action {
        Ok("<led>\n<pwm_set n=\"1\" r=\"255\" g=\"125\" b=\"3\"/>\n</led>")
    }

    def stream = Action {
        val file = new File("public/0233.ogg")
        val data = new FileInputStream(file)
        val dataContent: Enumerator[Array[Byte]] = Enumerator.fromStream(data)
            
        Ok.chunked(dataContent).
            withHeaders( (CONTENT_TYPE, "audio/ogg"),
                         (CACHE_CONTROL, "no-cache"))
    }
}
