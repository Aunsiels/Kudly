name := """kudly-server"""

version := "1.0-SNAPSHOT"

lazy val root = (project in file(".")).enablePlugins(PlayScala)

scalaVersion := "2.11.6"

libraryDependencies ++= Seq(
  jdbc,
  anorm,
  cache,
  ws,
  "org.mongodb" %% "casbah" % "2.8.0",
  "com.typesafe.play" %% "play-mailer" % "2.4.1"
)

scalacOptions += "-feature"
