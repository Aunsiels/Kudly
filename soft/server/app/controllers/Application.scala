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
import com.mongodb.casbah.gridfs.Imports._

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
        val data = rawCollection.findOne("data" $eq "led")
        data match {
            case Some(led) => {
                var l = Led (
                    led.getAs[Int]("n").getOrElse(0),
                    led.getAs[Int]("r").getOrElse(0),
                    led.getAs[Int]("g").getOrElse(0),
                    led.getAs[Int]("b").getOrElse(0))
                Ok("<led>\n<pwm_set n=\""+ 
                   + l.n + "\" r=\""
                   + l.r + "\" g=\""
                   + l.g + "\" b=\""
                   + l.b + "\"/>\n</led>")}
            case None =>
                Ok("No value yet")
        }
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
     * Set the pwm value
     */
    def setPwm = Action { implicit request =>
        ledForm.bindFromRequest.fold(
            errors => {
                BadRequest("Wrong parameters")},
            led    => {
                rawCollection.remove("data" $eq "led")
                var ledData = MongoDBObject(
                    "data"  -> "led",
                    "n" -> led.n,
                    "r" -> led.r,
                    "g" -> led.g,
                    "b" -> led.b
                )
                rawCollection += ledData
                Ok("Led data received")
            }
        )
    }

    /*
     * Graph data form
     */
    val graphForm : Form[Int] = Form(
         "value" -> number
    )

    /* 
     * Set temp value
     */
    def temp = Action { implicit request =>
        graphForm.bindFromRequest.fold(
            error => BadRequest("Bad argument"),
            data  => {
                var tempData = MongoDBObject(
                    "data"  -> "temp",
                    "value" -> data,
                    "date"  -> new java.util.Date()
                )
                rawCollection += tempData
                Ok("Temperature received")
            }
        )
    }

    /* Gridfs reference */
    val gridfs = GridFS(dataBase)

    /*
     * Test the writting/reading in the database
     */
    def testGridFS = {
        gridfs remove "mongodb_test.ogg"

        /* Save in the database */
        var image = new FileInputStream("public/0233.ogg")
        var id = gridfs(image) { f =>
            f.filename = "mongodb_test.ogg"
        }

        var imageReceived = gridfs.findOne("mongodb_test.ogg")
        imageReceived match {
            case Some(im) => println(im.filename)
            case None     => println("Nothing")
        }

        gridfs remove "mongodb_test.ogg"
    }

    /*
     * Uploads a file in the database
     */
    def upload = Action(parse.multipartFormData) { request =>
        request.body.file("picture").map { picture =>
            val filename = picture.filename 
            val contentType = picture.contentType

            picture.ref.moveTo(new File(s"/tmp/$filename"))
            var image = new FileInputStream(s"/tmp/$filename")

            /* Write in the database */
            var id = gridfs(image) { f =>
                f.filename = filename
                f.contentType = contentType.getOrElse("image/jpg")
            }
            
            Ok("File uploaded")
        }.getOrElse {
            Ok("Problem while upload")
        }
    }

    /*
     * Page to send an image
     */
    def sendimage = Action {
        Ok(views.html.image())
    }
}
