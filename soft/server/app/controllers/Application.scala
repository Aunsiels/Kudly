package controllers

import play.api._
import play.api.mvc._
import play.api.data._
import play.api.data.Forms._
import java.io.File
import java.io.FileInputStream
import play.api.libs.iteratee._
import scala.concurrent.ExecutionContext.Implicits.global
import play.api.mvc._
import play.api.Play.current
import akka.actor._

import com.mongodb.casbah.Imports._

object Application extends Controller {

    /* Database */
    val dataBase = MongoClient(
        MongoClientURI(
            "mongodb://kudly:19071993@ds041168.mongolab.com:41168/kudly"
        )
    ).getDB("kudly")

    /* Collection of raw data */
    val rawCollection = dataBase("Raw")

    /*
     * Welcome page
     */
    def index = Action {
        Ok("Welcome page")
    }
  
    /*
     * Connexion fake through url
     */
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

    /*
     * Stream an audio file throught a GET request
     */
    def stream = Action {
        val file = new File("public/0233.ogg")
        val data = new FileInputStream(file)
        val dataContent: Enumerator[Array[Byte]] = Enumerator.fromStream(data)
            
        Ok.chunked(dataContent).
            withHeaders( (CONTENT_TYPE, "audio/ogg"),
                         (CACHE_CONTROL, "no-cache"))
    }

    /*
     * Echo actor for websocket
     */
    object echoWSActor {
        def props(out: ActorRef) = Props(new echoWSActor(out))
    }

    /* echo actor */
    class echoWSActor(out: ActorRef) extends Actor {
        def receive = {
            case msg: String =>
                      out ! (msg)
        }
    }

    /* echo websocket */
    def echo = WebSocket.acceptWithActor[String, String] { request => out =>
        echoWSActor.props(out)
    }

    /* Echo Form */
    val echoForm : Form[String] = Form(
        "data" -> text
    )

    /* 
     * Simply answer with what was received
     */
    def postEcho = Action { implicit request =>
        echoForm.bindFromRequest.fold(
            errors => {
                BadRequest("Bad data") },
            data   => {
                Ok("Receive data : " + data)
            }
        )
    }

    /*
     * Contains a led description
     */
    case class Led (
        n     : Int,
        r     : Int,
        g     : Int,
        b     : Int
    )

    /*
     * Led form
     */
    val ledForm : Form[Led] = Form(
        mapping(
            "n" -> number(min = 0, max = 2),
            "r" -> number(min = 0, max = 255),
            "g" -> number(min = 0, max = 255),
            "b" -> number(min = 0, max = 255)
        )(Led.apply)(Led.unapply)
    )

    /*
     * Set the pwm value
     */
    def setPwm = Action { implicit request =>
        ledForm.bindFromRequest.fold(
            errors => {
                BadRequest("Wrong parameters")},
            led    => {
                rawCollection.remove("data" $eq "led")
                var ledData = MongoDBObject(
                    "data" -> "led",
                    "n"    -> led.n,
                    "r"    -> led.r,
                    "g"    -> led.g,
                    "b"    -> led.b)
                rawCollection += ledData
                Ok("Led data received")
            }
        )
    }


                 
}
