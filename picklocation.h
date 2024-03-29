#ifndef PICKLOCATION_H
#define PICKLOCATION_H

#include <QButtonGroup>
#include <QDialog>

namespace Ui
{
class PickLocation;
}

class PickLocation : public QDialog
{
    Q_OBJECT

public:
    explicit PickLocation(const QString &location, QWidget *parent = nullptr);
    ~PickLocation() override;
    QString button;

private slots:
    void onGroupButton(int button_id);

private:
    Ui::PickLocation *ui;
    QButtonGroup *buttonGroup;
};

#endif // PICKLOCATION_H
