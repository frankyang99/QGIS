/***************************************************************************
                         testqgstemporalnavigationobject.cpp
                         ---------------
    begin                : April 2020
    copyright            : (C) 2020 by Samweli Mwakisambwe
    email                : samweli at kartoza dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgstest.h"
#include <QObject>

//qgis includes...
#include <qgstemporalnavigationobject.h>

/**
 * \ingroup UnitTests
 * This is a unit test for the QgsTemporalNavigationObject class.
 */
class TestQgsTemporalNavigationObject : public QObject
{
    Q_OBJECT

  public:
    TestQgsTemporalNavigationObject() = default;

  private slots:
    void initTestCase();// will be called before the first testfunction is executed.
    void cleanupTestCase();// will be called after the last testfunction was executed.
    void init(); // will be called before each testfunction is executed.
    void cleanup(); // will be called after every testfunction.

    void animationState();
    void temporalExtents();
    void frameSettings();

  private:
    QgsTemporalNavigationObject *navigationObject = nullptr;
};

void TestQgsTemporalNavigationObject::initTestCase()
{
  //
  // Runs once before any tests are run
  //
  // init QGIS's paths - true means that all path will be inited from prefix
  QgsApplication::init();
  QgsApplication::initQgis();
  QgsApplication::showSettings();

}

void TestQgsTemporalNavigationObject::init()
{
  //create some objects that will be used in all tests...
  //create a temporal object that will be used in all tests...

  navigationObject = new QgsTemporalNavigationObject();
}

void TestQgsTemporalNavigationObject::cleanup()
{
}

void TestQgsTemporalNavigationObject::cleanupTestCase()
{
  QgsApplication::exitQgis();
}

void TestQgsTemporalNavigationObject::animationState()
{
  QgsDateTimeRange range = QgsDateTimeRange(
                             QDateTime( QDate( 2020, 1, 1 ), QTime( 8, 0, 0 ) ),
                             QDateTime( QDate( 2020, 10, 1 ), QTime( 8, 0, 0 ) )
                           );
  navigationObject->setTemporalExtents( range );

  navigationObject->setFrameDuration( QgsInterval( 1, QgsUnitTypes::TemporalMonths ) );

  qRegisterMetaType<QgsTemporalNavigationObject::AnimationState>( "AnimationState" );
  QSignalSpy stateSignal( navigationObject, &QgsTemporalNavigationObject::stateChanged );

  QCOMPARE( navigationObject->animationState(), QgsTemporalNavigationObject::Idle );

  navigationObject->setAnimationState( QgsTemporalNavigationObject::Forward );
  QCOMPARE( navigationObject->animationState(), QgsTemporalNavigationObject::Forward );
  QCOMPARE( stateSignal.count(), 1 );

  navigationObject->playBackward();
  QCOMPARE( navigationObject->animationState(), QgsTemporalNavigationObject::Reverse );
  QCOMPARE( stateSignal.count(), 2 );

  navigationObject->playForward();
  QCOMPARE( navigationObject->animationState(), QgsTemporalNavigationObject::Forward );
  QCOMPARE( stateSignal.count(), 3 );

  navigationObject->pause();
  QCOMPARE( navigationObject->animationState(), QgsTemporalNavigationObject::Idle );
  QCOMPARE( stateSignal.count(), 4 );

  navigationObject->next();
  QCOMPARE( navigationObject->currentFrameNumber(), 1 );

  navigationObject->previous();
  QCOMPARE( navigationObject->currentFrameNumber(), 0 );

  navigationObject->skipToEnd();
  QCOMPARE( navigationObject->currentFrameNumber(), 10 );

  navigationObject->rewindToStart();
  QCOMPARE( navigationObject->currentFrameNumber(), 0 );

  QCOMPARE( navigationObject->isLooping(), false );
  navigationObject->setLooping( true );
  QCOMPARE( navigationObject->isLooping(), true );

}

void TestQgsTemporalNavigationObject::temporalExtents()
{
  QgsDateTimeRange range = QgsDateTimeRange(
                             QDateTime( QDate( 2020, 1, 1 ), QTime( 8, 0, 0 ) ),
                             QDateTime( QDate( 2020, 12, 1 ), QTime( 8, 0, 0 ) )
                           );
  navigationObject->setTemporalExtents( range );
  QCOMPARE( navigationObject->temporalExtents(), range );

  navigationObject->setTemporalExtents( QgsDateTimeRange() );
  QCOMPARE( navigationObject->temporalExtents(), QgsDateTimeRange() );
}

void TestQgsTemporalNavigationObject::frameSettings()
{
  qRegisterMetaType<QgsDateTimeRange>( "QgsDateTimeRange" );
  QSignalSpy temporalRangeSignal( navigationObject, &QgsTemporalNavigationObject::updateTemporalRange );

  QgsDateTimeRange range = QgsDateTimeRange(
                             QDateTime( QDate( 2020, 1, 1 ), QTime( 8, 0, 0 ) ),
                             QDateTime( QDate( 2020, 1, 1 ), QTime( 12, 0, 0 ) )
                           );
  navigationObject->setTemporalExtents( range );
  QCOMPARE( temporalRangeSignal.count(), 1 );

  navigationObject->setFrameDuration( QgsInterval( 1, QgsUnitTypes::TemporalHours ) );
  QCOMPARE( navigationObject->frameDuration(), QgsInterval( 1, QgsUnitTypes::TemporalHours ) );

  QCOMPARE( navigationObject->currentFrameNumber(), 0 );
  QCOMPARE( navigationObject->totalFrameCount(), 5 );

  navigationObject->setCurrentFrameNumber( 1 );
  QCOMPARE( navigationObject->currentFrameNumber(), 1 );
  QCOMPARE( temporalRangeSignal.count(), 2 );

  navigationObject->setFramesPerSecond( 1 );
  QCOMPARE( navigationObject->framesPerSecond(), 1.0 );

}

QGSTEST_MAIN( TestQgsTemporalNavigationObject )
#include "testqgstemporalnavigationobject.moc"
