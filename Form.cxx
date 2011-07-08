/*=========================================================================
 *
 *  Copyright David Doria 2011 daviddoria@gmail.com
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *         http://www.apache.org/licenses/LICENSE-2.0.txt
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 *=========================================================================*/

#include "ui_Form.h"
#include "Form.h"

// ITK
#include "itkCastImageFilter.h"
#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"
#include "itkRegionOfInterestImageFilter.h"
#include "itkVector.h"
#include "itkDeformationFieldSource.h"
#include "itkDeformationFieldTransform.h"
#include "itkResampleVectorImageFilter.h"

// Qt
#include <QFileDialog>
#include <QIcon>

// VTK
#include <vtkActor.h>
#include <vtkCommand.h>
#include <vtkDataSetSurfaceFilter.h>
#include <vtkImageActor.h>
#include <vtkImageData.h>
#include <vtkInteractorStyleImage.h>
#include <vtkMath.h>
#include <vtkPointData.h>
#include <vtkProperty2D.h>
#include <vtkPolyDataMapper.h>
#include <vtkPoints.h>
#include <vtkPolyData.h>
#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkSeedRepresentation.h>
#include <vtkSeedWidget.h>
#include <vtkSmartPointer.h>
#include <vtkVertexGlyphFilter.h>

// Custom
#include "Helpers.h"
#include "Types.h"

// Constructor
Form::Form()
{
  this->setupUi(this);

  this->LeftRenderer = vtkSmartPointer<vtkRenderer>::New();
  this->RightRenderer = vtkSmartPointer<vtkRenderer>::New();

  std::cout << "Left renderer: " << this->LeftRenderer << std::endl;
  std::cout << "Right renderer: " << this->RightRenderer << std::endl;
  
  this->qvtkWidgetLeft->GetRenderWindow()->AddRenderer(this->LeftRenderer);
  this->qvtkWidgetRight->GetRenderWindow()->AddRenderer(this->RightRenderer);

  this->MovingImageActor = vtkSmartPointer<vtkImageActor>::New();
  this->MovingImageData = vtkSmartPointer<vtkImageData>::New();
  
  this->FixedImageActor = vtkSmartPointer<vtkImageActor>::New();
  this->FixedImageData = vtkSmartPointer<vtkImageData>::New();
  
  this->TransformedImageActor = vtkSmartPointer<vtkImageActor>::New();
  this->TransformedImageData = vtkSmartPointer<vtkImageData>::New();
  
  // Setup toolbar
  QIcon openIcon = QIcon::fromTheme("document-open");
  actionOpenFixedImage->setIcon(openIcon);
  this->toolBar->addAction(actionOpenFixedImage);
  
  actionOpenMovingImage->setIcon(openIcon);
  this->toolBar->addAction(actionOpenMovingImage);
  
  QIcon saveIcon = QIcon::fromTheme("document-save");
  actionSave->setIcon(saveIcon);
  this->toolBar->addAction(actionSave);
    
  this->FixedHandleRepresentation = vtkSmartPointer<vtkPointHandleRepresentation2D>::New();
  this->FixedHandleRepresentation->GetProperty()->SetColor(1,0,0);
  this->FixedSeedRepresentation = vtkSmartPointer<vtkSeedRepresentation>::New();
  this->FixedSeedRepresentation->SetHandleRepresentation(this->FixedHandleRepresentation);
  
  this->MovingHandleRepresentation = vtkSmartPointer<vtkPointHandleRepresentation2D>::New();
  this->MovingHandleRepresentation->GetProperty()->SetColor(1,0,0);
  this->MovingSeedRepresentation = vtkSmartPointer<vtkSeedRepresentation>::New();
  this->MovingSeedRepresentation->SetHandleRepresentation(this->MovingHandleRepresentation);
};

void Form::on_btnRegister_clicked()
{
  if(this->MovingSeedRepresentation->GetNumberOfSeeds() !=
     this->FixedSeedRepresentation->GetNumberOfSeeds())
  {
    std::cerr << "The number of fixed seeds must match the number of moving seeds!" << std::endl;
    return;
  }
  
  //typedef   itk::Vector< float, 2 >    VectorType;
  typedef   itk::Vector< double, 2 >    VectorType;
  typedef   itk::Image< VectorType, 2 >   DeformationFieldType;
  
  typedef itk::DeformationFieldSource<DeformationFieldType>  DeformationFieldSourceType;
  DeformationFieldSourceType::Pointer deformationFieldSource = DeformationFieldSourceType::New();
  deformationFieldSource->SetOutputSpacing( this->FixedImage->GetSpacing() );
  deformationFieldSource->SetOutputOrigin(  this->FixedImage->GetOrigin() );
  deformationFieldSource->SetOutputRegion(  this->FixedImage->GetLargestPossibleRegion() );
  deformationFieldSource->SetOutputDirection( this->FixedImage->GetDirection() );

  //  Create source and target landmarks.
  typedef DeformationFieldSourceType::LandmarkContainerPointer   LandmarkContainerPointer;
  typedef DeformationFieldSourceType::LandmarkContainer          LandmarkContainerType;
  typedef DeformationFieldSourceType::LandmarkPointType          LandmarkPointType;

  LandmarkContainerType::Pointer fixedLandmarks = LandmarkContainerType::New();
  LandmarkContainerType::Pointer movingLandmarks = LandmarkContainerType::New();

  LandmarkPointType movingPoint;
  LandmarkPointType fixedPoint;

  for(vtkIdType i = 0; i < this->FixedSeedRepresentation->GetNumberOfSeeds(); i++)
    {
    double fixedPos[3];
    this->FixedSeedRepresentation->GetSeedDisplayPosition(i, fixedPos);
    fixedPoint[0] = 40;
    fixedPoint[1] = 40;
    fixedLandmarks->InsertElement( i, fixedPoint );
  
    double movingPos[3];
    this->MovingSeedRepresentation->GetSeedDisplayPosition(i, movingPos);
    movingPoint[0] = 20;
    movingPoint[1] = 20;
    movingLandmarks->InsertElement( i, movingPoint );
    }

  deformationFieldSource->SetSourceLandmarks( movingLandmarks.GetPointer() );
  deformationFieldSource->SetTargetLandmarks( fixedLandmarks.GetPointer() );
  deformationFieldSource->UpdateLargestPossibleRegion();
  
  //typedef itk::DeformationFieldTransform<float, 2>  DeformationFieldTransformType;
  typedef itk::DeformationFieldTransform<double, 2>  DeformationFieldTransformType;
  DeformationFieldTransformType::Pointer deformationFieldTransform = DeformationFieldTransformType::New();
  deformationFieldTransform->SetDeformationField( deformationFieldSource->GetOutput() );
  
  //typedef itk::ResampleVectorImageFilter<FloatVectorImageType, FloatVectorImageType, float >    VectorResampleFilterType;
  typedef itk::ResampleVectorImageFilter<FloatVectorImageType, FloatVectorImageType>    VectorResampleFilterType;
  VectorResampleFilterType::Pointer vectorResampleFilter = VectorResampleFilterType::New();
  vectorResampleFilter->SetInput( this->MovingImage );
  vectorResampleFilter->SetTransform( deformationFieldTransform );
  vectorResampleFilter->SetSize( this->FixedImage->GetLargestPossibleRegion().GetSize() );
  vectorResampleFilter->SetOutputOrigin(  this->FixedImage->GetOrigin() );
  vectorResampleFilter->SetOutputSpacing( this->FixedImage->GetSpacing() );
  vectorResampleFilter->SetOutputDirection( this->FixedImage->GetDirection() );
  vectorResampleFilter->SetDefaultPixelValue( 200 ); // This is the color which to set portions of the transformed image that do not correspond to the moving image
  vectorResampleFilter->Update();
  
  this->TransformedImage = FloatVectorImageType::New();
  Helpers::DeepCopyVectorImage<FloatVectorImageType>(vectorResampleFilter->GetOutput(), this->TransformedImage);
    
  if(this->chkRGB->isChecked())
    {
    Helpers::ITKImagetoVTKRGBImage(this->TransformedImage, this->TransformedImageData);
    }
  else
    {
    Helpers::ITKImagetoVTKMagnitudeImage(this->TransformedImage, this->TransformedImageData);
    }
  
  this->TransformedImageActor->SetInput(this->TransformedImageData);

  // Add Actor to renderer
  this->LeftRenderer->AddActor(this->TransformedImageActor);
  
  this->qvtkWidgetLeft->GetInteractor()->GetRenderWindow()->Render();
  this->LeftRenderer->Render();
  
  //this->LeftRenderer->ResetCamera();
}

void Form::on_actionOpenMovingImage_activated()
{
   // Get a filename to open
  QString fileName = QFileDialog::getOpenFileName(this, "Open File", ".", "Image Files (*.png *.mhd *.tif)");

  std::cout << "Got filename: " << fileName.toStdString() << std::endl;
  if(fileName.toStdString().empty())
    {
    std::cout << "Filename was empty." << std::endl;
    return;
    }

  typedef itk::ImageFileReader<FloatVectorImageType> ReaderType;
  ReaderType::Pointer reader = ReaderType::New();
  reader->SetFileName(fileName.toStdString());
  reader->Update();
  
  this->MovingImage = reader->GetOutput();

  if(this->chkRGB->isChecked())
    {
    Helpers::ITKImagetoVTKRGBImage(this->MovingImage, this->MovingImageData);
    }
  else
    {
    Helpers::ITKImagetoVTKMagnitudeImage(this->MovingImage, this->MovingImageData);
    }
  
  this->MovingImageActor->SetInput(this->MovingImageData);

  // Add Actor to renderer
  this->RightRenderer->AddActor(this->MovingImageActor);
  this->RightRenderer->ResetCamera();

  vtkSmartPointer<vtkInteractorStyleImage> interactorStyle =
      vtkSmartPointer<vtkInteractorStyleImage>::New();
  this->qvtkWidgetRight->GetRenderWindow()->GetInteractor()->SetInteractorStyle(interactorStyle);

  this->RightRenderer->ResetCamera();

  
  // Seed widget
  this->MovingSeedWidget = vtkSmartPointer<vtkSeedWidget>::New();
  this->MovingSeedWidget->SetInteractor(this->qvtkWidgetRight->GetRenderWindow()->GetInteractor());
  this->MovingSeedWidget->SetRepresentation(this->MovingSeedRepresentation);
  
  this->MovingSeedCallback = vtkSmartPointer<vtkSeedCallback>::New();
  this->MovingSeedCallback->SetWidget(this->MovingSeedWidget);
  
  this->MovingSeedWidget->AddObserver(vtkCommand::PlacePointEvent,this->MovingSeedCallback);
  this->MovingSeedWidget->AddObserver(vtkCommand::InteractionEvent,this->MovingSeedCallback);
  this->MovingSeedWidget->On();
}

void Form::on_actionOpenFixedImage_activated()
{
  // Get a filename to open
  QString fileName = QFileDialog::getOpenFileName(this, "Open File", ".", "Image Files (*.png *.mhd *.tif)");

  std::cout << "Got filename: " << fileName.toStdString() << std::endl;
  if(fileName.toStdString().empty())
    {
    std::cout << "Filename was empty." << std::endl;
    return;
    }

  typedef itk::ImageFileReader<FloatVectorImageType> ReaderType;
  ReaderType::Pointer reader = ReaderType::New();
  reader->SetFileName(fileName.toStdString());
  reader->Update();
  
  this->FixedImage = reader->GetOutput();

  if(this->chkRGB->isChecked())
    {
    Helpers::ITKImagetoVTKRGBImage(this->FixedImage, this->FixedImageData);
    }
  else
    {
    Helpers::ITKImagetoVTKMagnitudeImage(this->FixedImage, this->FixedImageData);
    }
  
  this->FixedImageActor->SetInput(this->FixedImageData);

  // Add Actor to renderer
  this->LeftRenderer->AddActor(this->FixedImageActor);
  this->LeftRenderer->ResetCamera();

  vtkSmartPointer<vtkInteractorStyleImage> interactorStyle =
      vtkSmartPointer<vtkInteractorStyleImage>::New();
  this->qvtkWidgetLeft->GetRenderWindow()->GetInteractor()->SetInteractorStyle(interactorStyle);

  this->LeftRenderer->ResetCamera();

  std::cout << "Fixed interactor: " << this->qvtkWidgetLeft->GetRenderWindow()->GetInteractor() << std::endl;
  // Seed widget
  this->FixedSeedWidget = vtkSmartPointer<vtkSeedWidget>::New();
  this->FixedSeedWidget->SetInteractor(this->qvtkWidgetLeft->GetRenderWindow()->GetInteractor());
  this->FixedSeedWidget->SetRepresentation(this->FixedSeedRepresentation);
  
  this->FixedSeedCallback = vtkSmartPointer<vtkSeedCallback>::New();
  this->FixedSeedCallback->SetWidget(this->FixedSeedWidget);
  
  this->FixedSeedWidget->AddObserver(vtkCommand::PlacePointEvent,this->FixedSeedCallback);
  this->FixedSeedWidget->AddObserver(vtkCommand::InteractionEvent,this->FixedSeedCallback);
  this->FixedSeedWidget->On();
}

void Form::on_actionSave_activated()
{
  if(this->chkRGB->isChecked())
    {
    QString fileName = QFileDialog::getSaveFileName(this, "Save File", ".", "Image Files (*.png)");
    std::cout << "Got filename: " << fileName.toStdString() << std::endl;
    if(fileName.toStdString().empty())
      {
      std::cout << "Filename was empty." << std::endl;
      return;
      }
    typedef itk::CastImageFilter< FloatVectorImageType, UnsignedCharVectorImageType > CastFilterType;
    CastFilterType::Pointer castFilter = CastFilterType::New();
    castFilter->SetInput(this->TransformedImage);
    castFilter->Update();
    
    typedef  itk::ImageFileWriter< UnsignedCharVectorImageType  > WriterType;
    WriterType::Pointer writer = WriterType::New();
    writer->SetFileName(fileName.toStdString());
    writer->SetInput(castFilter->GetOutput());
    writer->Update();
    }
  else
    {
    QString fileName = QFileDialog::getSaveFileName(this, "Save File", ".", "Image Files (*.mhd)");
    std::cout << "Got filename: " << fileName.toStdString() << std::endl;
    if(fileName.toStdString().empty())
      {
      std::cout << "Filename was empty." << std::endl;
      return;
      }
    typedef  itk::ImageFileWriter< FloatVectorImageType  > WriterType;
    WriterType::Pointer writer = WriterType::New();
    writer->SetFileName(fileName.toStdString());
    writer->SetInput(this->TransformedImage);
    writer->Update();
    }

}
