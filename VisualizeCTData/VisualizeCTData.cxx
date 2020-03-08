#include "VisualizeCTData.h"

#include <QFileDialog>

#include "InteractorStyleDisplayVolume.h"
#include <vtkDICOMImageReader.h>

#include <vtkRenderWindowInteractor.h>
#include <vtkRenderer.h>
#include <vtkInteractorStyleUser.h>
#include <vtkCamera.h>

#include <vtkGenericOpenGLRenderWindow.h>
#include <vtkCallbackCommand.h>
#include <vtkNamedColors.h>
#include <vtkNew.h>
#include <vtkRenderWindow.h>
#include <vtkRenderer.h>
#include <vtkSmartPointer.h>


// Constructor
VisualizeCTData::VisualizeCTData()
{
	this->setupUi(this);
	vtkNew<vtkNamedColors> colors;

	renderWindow = vtkSmartPointer<vtkGenericOpenGLRenderWindow>::New();
	renderWindow->SetWindowName("VisualizeCTData");

	renderer = vtkSmartPointer<vtkRenderer>::New();
	renderer->SetBackground(colors->GetColor3d("SteelBlue").GetData());
	renderWindow->AddRenderer(renderer);
	qvtkWidget->SetRenderWindow(renderWindow);

	renderWindowInteractor = vtkSmartPointer<vtkRenderWindowInteractor>::New();
	renderWindowInteractor->SetRenderWindow(renderWindow);

	style = vtkSmartPointer<InteractorStyleDisplayVolume>::New();
	style->SetCurrentRenderer(renderer);
	renderWindowInteractor->SetInteractorStyle(style);

	renderWindow->Render();
	renderWindowInteractor->Initialize();

	connect(pushButtonLoad, &QPushButton::clicked, this, &VisualizeCTData::LoadDICOM);
};

void VisualizeCTData::LoadDICOM()
{
	QString dir = QFileDialog::getExistingDirectory(this, tr("Open Directory"),"/",
		QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);

	if (dir.count() > 0)
	{
		dicomReader = vtkSmartPointer<vtkDICOMImageReader>::New();
		dicomReader->SetDirectoryName(dir.toStdString().c_str());
		dicomReader->SetDataByteOrderToLittleEndian();
		dicomReader->Update();

		this->style->GenerateVolumePreview(*dicomReader->GetOutput());
	}

}
