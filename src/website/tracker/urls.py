from django.conf.urls import url 

from . import views

urlpatterns = [
    url(r'^get_time/', views.get_time, name='get_time'),
    url('', views.index, name='index'),
]
