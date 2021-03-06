/***************************************************************************
                         qgsmeshlayer.h
                         --------------
    begin                : April 2018
    copyright            : (C) 2018 by Peter Petrik
    email                : zilolv at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSMESHLAYER_H
#define QGSMESHLAYER_H

#include <memory>

#include "qgis_core.h"
#include "qgsmaplayer.h"
#include "qgsmeshdataprovider.h"
#include "qgsmeshrenderersettings.h"
#include "qgsmeshtimesettings.h"
#include "qgsmeshsimplificationsettings.h"
#include "qgsmeshlayertemporalproperties.h"

class QgsMapLayerRenderer;
struct QgsMeshLayerRendererCache;
class QgsSymbol;
class QgsTriangularMesh;
class QgsRenderContext;
struct QgsMesh;
class QgsMesh3dAveragingMethod;

/**
 * \ingroup core
 *
 * Represents a mesh layer supporting display of data on structured or unstructured meshes
 *
 * The QgsMeshLayer is instantiated by specifying the name of a data provider,
 * such as mdal, and url defining the specific data set to connect to.
 * The vector layer constructor in turn instantiates a QgsMeshDataProvider subclass
 * corresponding to the provider type, and passes it the url. The data provider
 * connects to the data source.
 *
 * The QgsMeshLayer provides a common interface to the different data types. It does not
 * yet support editing transactions.
 *
 * The main data providers supported by QGIS are listed below.
 *
 * \section mesh_providers Mesh data providers
 *
 * \subsection mesh_memory Memory data providerType (mesh_memory)
 *
 * The memory data provider is used to construct in memory data, for example scratch
 * data. There is no inherent persistent storage of the data. The data source uri is constructed.
 * Data can be populated by setMesh(const QString &vertices, const QString &faces), where
 * vertices and faces is comma separated coordinates and connections for mesh.
 * E.g. to create mesh with one quad and one triangle
 *
 * \code
 *  QString uri(
 *      "1.0, 2.0 \n" \
 *      "2.0, 2.0 \n" \
 *      "3.0, 2.0 \n" \
 *      "2.0, 3.0 \n" \
 *      "1.0, 3.0 \n" \
 *      "---" \
 *      "0, 1, 3, 4 \n" \
 *      "1, 2, 3 \n"
 *    );
 *    QgsMeshLayer *scratchLayer = new QgsMeshLayer(uri, "My Scratch layer", "mesh_memory");
 * \endcode
 *
 * \subsection mdal MDAL data provider (mdal)
 *
 * Accesses data using the MDAL drivers (https://github.com/lutraconsulting/MDAL). The url
 * is the MDAL connection string. QGIS must be built with MDAL support to allow this provider.

 * \code
 *     QString uri = "test/land.2dm";
 *     QgsMeshLayer *scratchLayer = new QgsMeshLayer(uri, "My Scratch Layer",  "mdal");
 * \endcode
 *
 * \note The API is considered EXPERIMENTAL and can be changed without a notice
 *
 * \since QGIS 3.2
 */
class CORE_EXPORT QgsMeshLayer : public QgsMapLayer
{
    Q_OBJECT
  public:

    /**
     * Setting options for loading mesh layers.
     */
    struct LayerOptions
    {

      /**
       * Constructor for LayerOptions with optional \a transformContext.
       * \note transformContext argument was added in QGIS 3.8
       */
      explicit LayerOptions( const QgsCoordinateTransformContext &transformContext = QgsCoordinateTransformContext( ) )
        : transformContext( transformContext )
      {}

      QgsCoordinateTransformContext transformContext;

      /**
       * Controls whether the layer is allowed to have an invalid/unknown CRS.
       *
       * If TRUE, then no validation will be performed on the layer's CRS and the layer
       * layer's crs() may be invalid() (i.e. the layer will have no georeferencing available
       * and will be treated as having purely numerical coordinates).
       *
       * If FALSE (the default), the layer's CRS will be validated using QgsCoordinateReferenceSystem::validate(),
       * which may cause a blocking, user-facing dialog asking users to manually select the correct CRS for the
       * layer.
       *
       * \since QGIS 3.10
       */
      bool skipCrsValidation = false;
    };

    /**
     * Constructor - creates a mesh layer
     *
     * The QgsMeshLayer is constructed by instantiating a data provider.  The provider
     * interprets the supplied path (url) of the data source to connect to and access the
     * data.
     *
     * \param path  The path or url of the parameter.  Typically this encodes
     *               parameters used by the data provider as url query items.
     * \param baseName The name used to represent the layer in the legend
     * \param providerLib  The name of the data provider, e.g., "mesh_memory", "mdal"
     * \param options general mesh layer options
     */
    explicit QgsMeshLayer( const QString &path = QString(), const QString &baseName = QString(), const QString &providerLib = QStringLiteral( "mesh_memory" ),
                           const QgsMeshLayer::LayerOptions &options = QgsMeshLayer::LayerOptions() );

    ~QgsMeshLayer() override;

    //! QgsMeshLayer cannot be copied.
    QgsMeshLayer( const QgsMeshLayer &rhs ) = delete;
    //! QgsMeshLayer cannot be copied.
    QgsMeshLayer &operator=( QgsMeshLayer const &rhs ) = delete;

    QgsMeshDataProvider *dataProvider() override;
    const QgsMeshDataProvider *dataProvider() const override SIP_SKIP;
    QgsMeshLayer *clone() const override SIP_FACTORY;
    QgsRectangle extent() const override;
    QgsMapLayerRenderer *createMapRenderer( QgsRenderContext &rendererContext ) override SIP_FACTORY;
    bool readSymbology( const QDomNode &node, QString &errorMessage,
                        QgsReadWriteContext &context, QgsMapLayer::StyleCategories categories = QgsMapLayer::AllStyleCategories ) override;
    bool writeSymbology( QDomNode &node, QDomDocument &doc, QString &errorMessage,
                         const QgsReadWriteContext &context, QgsMapLayer::StyleCategories categories = QgsMapLayer::AllStyleCategories ) const override;
    bool writeStyle( QDomNode &node, QDomDocument &doc, QString &errorMessage, const QgsReadWriteContext &context, StyleCategories categories = AllStyleCategories ) const override;
    bool readStyle( const QDomNode &node, QString &errorMessage, QgsReadWriteContext &context, StyleCategories categories = AllStyleCategories ) override;
    QString encodedSource( const QString &source, const QgsReadWriteContext &context ) const override;
    QString decodedSource( const QString &source, const QString &provider, const QgsReadWriteContext &context ) const override;
    bool readXml( const QDomNode &layer_node, QgsReadWriteContext &context ) override;
    bool writeXml( QDomNode &layer_node, QDomDocument &doc, const QgsReadWriteContext &context ) const override;
    QgsMeshLayerTemporalProperties *temporalProperties() override;
    void reload() override;
    QStringList subLayers() const override;

    //! Returns the provider type for this layer
    QString providerType() const;

    /**
     * Returns native mesh (NULLPTR before rendering or calling to updateMesh)
     *
     * \note Not available in Python bindings
     */
    QgsMesh *nativeMesh() SIP_SKIP;

    /**
     * Returns native mesh (NULLPTR before rendering or calling to updateMesh)
     *
     * \note Not available in Python bindings
     */
    const QgsMesh *nativeMesh() const SIP_SKIP;

    /**
     * Returns triangular mesh (NULLPTR before rendering or calling to updateMesh).
     *
     * If the parameter triangleSize is provided, among the base triangular mesh
     * and the simplified triangular meshes, the one returned is which has the average triangle size just greater than triangleSize.
     * The size of a triangle is the maximum between the height and the width of the triangle bounding box
     * For default parameter (=0), it returns base triangular mesh.
     * \param minimumTriangleSize is the average size criteria in canvas map units
     * \returns triangular mesh, the layer keeps the ownership
     * \note triangular size added in QGIS 3.14
     * \note Not available in Python bindings
     */
    QgsTriangularMesh *triangularMesh( double minimumTriangleSize = 0 ) const SIP_SKIP;

    /**
     * Gets native mesh and updates (creates if it doesn't exist) the base triangular mesh
     *
     * \param transform Transformation from layer CRS to destination (e.g. map) CRS. With invalid transform, it keeps the native mesh CRS
     *
     * \since QGIS 3.14
     */
    void updateTriangularMesh( const QgsCoordinateTransform &transform = QgsCoordinateTransform() );

    /**
     * Returns native mesh (NULLPTR before rendering)
     *
     * \note Not available in Python bindings
     */
    QgsMeshLayerRendererCache *rendererCache() SIP_SKIP;

    //! Returns renderer settings
    QgsMeshRendererSettings rendererSettings() const;
    //! Sets new renderer settings
    void setRendererSettings( const QgsMeshRendererSettings &settings );

    /**
     * Returns time format settings
     *
     * \since QGIS 3.8
     */
    QgsMeshTimeSettings timeSettings() const;

    /**
     * Sets time format settings
     *
     * \since QGIS 3.8
     */
    void setTimeSettings( const QgsMeshTimeSettings &settings );

    /**
     * Returns mesh simplification settings
     *
     * \since QGIS 3.14
     */
    QgsMeshSimplificationSettings meshSimplificationSettings() const SIP_SKIP;

    /**
     * Sets mesh simplification settings
     *
     * \since QGIS 3.14
     */
    void setMeshSimplificationSettings( const QgsMeshSimplificationSettings &meshSimplificationSettings ) SIP_SKIP;

    /**
     * Returns (date) time in hours formatted to human readable form
     * \param hours time in double in hours
     * \returns formatted time string
     * \since QGIS 3.8
     */
    QString formatTime( double hours );

    /**
      * Interpolates the value on the given point from given dataset.
      * For 3D datasets, it uses dataset3dValue(), \n
      * For 1D datasets, it uses dataset1dValue() with \a searchRadius
      *
      * \note It uses previously cached and indexed triangular mesh
      * and so if the layer has not been rendered previously
      * (e.g. when used in a script) it returns NaN value
      * \see updateTriangularMesh
      *
      * \param index dataset index specifying group and dataset to extract value from
      * \param point point to query in map coordinates
      * \param searchRadius the radius of the search area in map unit
      * \returns interpolated value at the point. Returns NaN values for values
      * outside the mesh layer, nodata values and in case triangular mesh was not
      * previously used for rendering
      *
      * \since QGIS 3.4
      */
    QgsMeshDatasetValue datasetValue( const QgsMeshDatasetIndex &index, const QgsPointXY &point, double searchRadius = 0 ) const;

    /**
      * Returns the 3d values of stacked 3d mesh defined by the given point
      *
      * \note It uses previously cached and indexed triangular mesh
      * and so if the layer has not been rendered previously
      * (e.g. when used in a script) it returns NaN value
      * \see updateTriangularMesh
      *
      * \param index dataset index specifying group and dataset to extract value from
      * \param point point to query in map coordinates
      * \returns all 3d stacked values that belong to face defined by given point. Returns invalid block
      * for point outside the mesh layer or in case triangular mesh was not
      * previously used for rendering or for datasets that do not have type DataOnVolumes
      *
      * \since QGIS 3.12
      */
    QgsMesh3dDataBlock dataset3dValue( const QgsMeshDatasetIndex &index, const QgsPointXY &point ) const;

    /**
      * Returns the value of 1D mesh dataset defined on edge that are in the search area defined by point ans searchRadius
      *
      * \note It uses previously cached and indexed triangular mesh
      * and so if the layer has not been rendered previously
      * (e.g. when used in a script) it returns NaN value
      * \see updateTriangularMesh
      *
      * \param index dataset index specifying group and dataset to extract value from
      * \param point the center point of the search area
      * \param searchRadius the radius of the searc area in map unit
      * \returns interpolated value at the projected point. Returns NaN values for values
      * outside the mesh layer and in case triangular mesh was not previously used for rendering
      *
      * \since QGIS 3.14
      */
    QgsMeshDatasetValue dataset1dValue( const QgsMeshDatasetIndex &index, const QgsPointXY &point, double searchRadius ) const;

    /**
      * Returns dataset index from active scalar group depending on the time range.
      * If the temporal properties is not active, return the static dataset
      *
      * \param timeRange the time range
      * \returns dataset index
      *
      * \since QGIS 3.14
      */
    QgsMeshDatasetIndex activeScalarDatasetAtTime( const QgsDateTimeRange &timeRange ) const;

    /**
      * Returns dataset index from active vector group depending on the time range
      * If the temporal properties is not active, return the static dataset
      *
      * \param timeRange the time range
      * \returns dataset index
      *
      * \since QGIS 3.14
      */
    QgsMeshDatasetIndex activeVectorDatasetAtTime( const QgsDateTimeRange &timeRange ) const;

    /**
      * Sets the static scalar dataset index that is rendered if the temporal properties is not active
      *
      * \param staticScalarDatasetIndex the scalar data set index
      *
      * \since QGIS 3.14
      */
    void setStaticScalarDatasetIndex( const QgsMeshDatasetIndex &staticScalarDatasetIndex ) SIP_SKIP;

    /**
      * Sets the static vector dataset index that is rendered if the temporal properties is not active
      *
      * \param staticVectorDatasetIndex the vector data set index
      *
      * \since QGIS 3.14
      */
    void setStaticVectorDatasetIndex( const QgsMeshDatasetIndex &staticVectorDatasetIndex ) SIP_SKIP;

    /**
      * Returns the static scalar dataset index that is rendered if the temporal properties is not active
      *
      * \since QGIS 3.14
      */
    QgsMeshDatasetIndex staticScalarDatasetIndex() const;

    /**
      * Returns the static vector dataset index that is rendered if the temporal properties is not active
      *
      * \since QGIS 3.14
      */
    QgsMeshDatasetIndex staticVectorDatasetIndex() const;

    /**
      * Sets the reference time of the layer
      *
      * \param referenceTime the reference time
      *
      * \since QGIS 3.14
      */
    void setReferenceTime( const QDateTime &referenceTime );

  public slots:

    /**
     * Sets the coordinate transform context to \a transformContext.
     *
     * \since QGIS 3.8
     */
    void setTransformContext( const QgsCoordinateTransformContext &transformContext ) override;

  signals:

    /**
     * Emitted when active scalar group dataset is changed
     *
     * \since QGIS 3.14
     */
    void activeScalarDatasetGroupChanged( int index );

    /**
     * Emitted when active vector group dataset is changed
     *
     * \since QGIS 3.14
     */
    void activeVectorDatasetGroupChanged( int index );

    /**
     * Emitted when time format is changed
     *
     * \since QGIS 3.8
     */
    void timeSettingsChanged( );

  private: // Private methods

    /**
     * Returns TRUE if the provider is in read-only mode
     */
    bool isReadOnly() const override {return true;}

    /**
     * Binds layer to a specific data provider
     * \param provider provider key string, must match a valid QgsMeshDataProvider key. E.g. "mesh_memory", etc.
     * \param options generic provider options
     */
    bool setDataProvider( QString const &provider, const QgsDataProvider::ProviderOptions &options );

#ifdef SIP_RUN
    QgsMeshLayer( const QgsMeshLayer &rhs );
#endif

  private:
    void fillNativeMesh();
    void assignDefaultStyleToDatasetGroup( int groupIndex );
    void setDefaultRendererSettings();
    void createSimplifiedMeshes();
    int levelsOfDetailsIndex( double partOfMeshInView ) const;

    bool hasSimplifiedMeshes() const;

    QgsMeshDatasetIndex datasetIndexAtTime( const QgsDateTimeRange &timeRange, int datasetGroupIndex ) const;

  private slots:
    void onDatasetGroupsAdded( int count );

  private:
    //! Pointer to data provider derived from the abastract base class QgsMeshDataProvider
    QgsMeshDataProvider *mDataProvider = nullptr;

    //! Pointer to native mesh structure, used as cache for rendering
    std::unique_ptr<QgsMesh> mNativeMesh;

    //! Pointer to derived mesh structures (the first one is the base mesh, others are simplified meshes with decreasing level of detail)
    std::vector<std::unique_ptr<QgsTriangularMesh>> mTriangularMeshes;

    //! Pointer to the cache with data used for last rendering
    std::unique_ptr<QgsMeshLayerRendererCache> mRendererCache;

    //! Renderer configuration
    QgsMeshRendererSettings mRendererSettings;

    //! Time format configuration
    QgsMeshTimeSettings mTimeSettings;

    //! Simplify mesh configuration
    QgsMeshSimplificationSettings mSimplificationSettings;

    QgsMeshLayerTemporalProperties *mTemporalProperties;

    QgsMeshDatasetIndex mStaticScalarDatasetIndex;
    QgsMeshDatasetIndex mStaticVectorDatasetIndex;
};

#endif //QGSMESHLAYER_H
